// Fill out your copyright notice in the Description page of Project Settings.


#include "Slash/Public/Characters/SlashCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h" 
#include "Components/AttributeComponent.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "GroomComponent.h" 
#include "Items/Item.h"
#include "Weapons/Weapon.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"


// Sets default values
ASlashCharacter::ASlashCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// set use controller rotation
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	// Create spring arm and attach to capsule
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	// Camera
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");
}

// Called to bind functionality to input
void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ASlashCharacter::MoveForward);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &ASlashCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Dodge);
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser) {
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector &ImpactPoint, AActor *Hitter) {
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

	if (IsAlive()) {
		ActionState = EActionState::EAS_HitReaction;
	}
}

// Called when the game starts or when spawned
void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(FName("EngageableTarget"));
	
	InitializeSlashOverlay();
	InitializeMappingContext();
}

void ASlashCharacter::Move(const FInputActionValue &Value) {
	if (ActionState != EActionState::EAS_Unoccupied) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	// This will move the character to its forward instead of the camera forward
	// if (GetController()) {
	// 	if (MovementVector.X != 0.f) {
	// 		FVector Forward = GetActorForwardVector();
	// 		// AddMovementInput only care if Value is +ve or -ve or nth 
	// 		AddMovementInput(Forward, MovementVector.X);
	// 	}	
	// 	if (MovementVector.Y != 0.f) {
	// 		FVector Right = GetActorRightVector();
	// 		AddMovementInput(Right, MovementVector.Y);
	// 	}
	// }

	// Move character forward the camera left/right/forward/back
	// find out which way is camera forward direction
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ASlashCharacter::Look(const FInputActionValue &Value) {
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void ASlashCharacter::Jump()
{
	if (IsUnoccupied()) {
		Super::Jump();
	}
}

void ASlashCharacter::Dodge() {}

void ASlashCharacter::EKeyPressed() {
	if (AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem)) {
		EquipWeapon(OverlappingWeapon);
	} else {
		if (CanDisarm()) {
			Disarm();
		} else if (CanArm()) { 
			Arm();
		}
	}
}

void ASlashCharacter::Attack() {
	Super::Attack();

	if (CanAttack()) {
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::EquipWeapon(AWeapon *Weapon)
{
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	EquippedWeapon = Weapon; 
	OverlappingItem = nullptr;
	if (Weapon->GetIsTwoHanded()) {
		CharacterState = ECharacterState::ESC_EquippedTwoHandedWeapon;
	} else {
		CharacterState = ECharacterState::ESC_EquippedOneHandedWeapon;
	}
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
    return CharacterState != ECharacterState::ECS_Unequipped &&
	 	ActionState == EActionState::EAS_Unoccupied;
}
bool ASlashCharacter::CanDisarm()
{
    return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}
bool ASlashCharacter::CanArm()
{
    return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		EquippedWeapon;
}

void ASlashCharacter::Disarm()
{
	ActionState = EActionState::EAS_EquippingWeapon;
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
}

void ASlashCharacter::Arm() {
	ActionState = EActionState::EAS_EquippingWeapon;
	PlayEquipMontage(FName("Equip"));
	CharacterState = ECharacterState::ESC_EquippedOneHandedWeapon;
}

void ASlashCharacter::LockEnemy(AActor* Enemy) {
	CombatTarget = Enemy;
}

void ASlashCharacter::Die() {
	Super::Die();
	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
}

void ASlashCharacter::AttachWeaponToBack() {
        if (EquippedWeapon) {
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}
void ASlashCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon) {
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}
void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::HitReactEnd() {
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::InitializeSlashOverlay() {
	PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController) {
		if (ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD())) {
			SlashOverlay = SlashHUD->GetSlashOverlay();
			if (SlashOverlay && Attributes) {
				SetHUDHealth();
				SlashOverlay->SetStaminaBarPercent(1.f);
				SlashOverlay->SetGold(0);
				SlashOverlay->SetSouls(0);
			}
		}
	}
}

void ASlashCharacter::InitializeMappingContext() {
	if (PlayerController) {
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem) {
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}
}

void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverlay && Attributes) {
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}
bool ASlashCharacter::IsUnoccupied() { return ActionState == EActionState::EAS_Unoccupied; }