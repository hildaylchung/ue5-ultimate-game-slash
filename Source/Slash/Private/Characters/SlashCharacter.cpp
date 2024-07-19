// Fill out your copyright notice in the Description page of Project Settings.


#include "Slash/Public/Characters/SlashCharacter.h"
#include "Components/InputComponent.h" 
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "GroomComponent.h" 
#include "Items/Item.h"
#include "Weapons/Weapon.h"
#include "Animation/AnimInstance.h"
#include "Components/BoxComponent.h"

// Sets default values
ASlashCharacter::ASlashCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// set use controller rotation
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

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

// Called when the game starts or when spawned
void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController())) {
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem) {
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}
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
	ACharacter::Jump();
}

void ASlashCharacter::Dodge() {}
void ASlashCharacter::EKeyPressed() {
	if (AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem)) {
		OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"));
		EquippedWeapon = OverlappingWeapon; 
		OverlappingItem = nullptr;
		if (OverlappingWeapon->GetIsTwoHanded()) {
			CharacterState = ECharacterState::ESC_EquippedTwoHandedWeapon;
		} else {
			CharacterState = ECharacterState::ESC_EquippedOneHandedWeapon;
		}
	} else {
		if (CanDisarm()) {
			PlayEquipMontage(FName("Unequip"));
			ActionState = EActionState::EAS_EquippingWeapon;
			CharacterState = ECharacterState::ECS_Unequipped;
		} else if (CanArm()) { 
			PlayEquipMontage(FName("Equip"));
			ActionState = EActionState::EAS_EquippingWeapon;
			CharacterState = ECharacterState::ESC_EquippedOneHandedWeapon;
		}
	}
	
}

void ASlashCharacter::Attack() {
	if (CanAttack()) {
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::PlayAttackMontage() {
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage) {
		AnimInstance->Montage_Play(AttackMontage);
		const int32 Selection = FMath::RandRange(0, 3);
		FString SectionNameString = FString::Printf(TEXT("Attack%d"), Selection + 1);
		FName SectionName = FName(SectionNameString);
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	}
}

void ASlashCharacter::PlayEquipMontage(FName SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage) {
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
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
	if (EquippedWeapon) {
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}
void ASlashCharacter::Arm()
{
	if (EquippedWeapon) {
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}
void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}


void ASlashCharacter::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
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

void ASlashCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox()) {
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->ResetIgnoreActors();
	}
}