// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Bird.h"
#include "Components/CapsuleComponent.h" 
#include "Components/SkeletalMeshComponent.h" 
#include "Components/InputComponent.h" 
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ABird::ABird()
{
	PrimaryActorTick.bCanEverTick = true;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));

	Capsule->SetCapsuleHalfHeight(20.f);
	Capsule->SetCapsuleRadius(15.f);

	// setting root component
	// Method 1: Same with Items.cpp
	// RootComponent = Capsule;
	
	// Method 2: with inherited setter method function
	SetRootComponent(Capsule);

	BirdMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BirdMesh"));
	// attach to root component
	// RootComponent/Capsule for first parameter also work
	BirdMesh->SetupAttachment(GetRootComponent());

	// Create spring arm and attach to capsule
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	// Camera
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);


	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ABird::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	
	// Also work if written like this:
	// ```
	// 	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	//  if (PlayerController) {...}
	// ```
	// but the scope of which PlayerController exists are different
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController())) {
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem) {
			Subsystem->AddMappingContext(BirdMappingContext, 0);
		}
	}
}

/// Move/Look Action for the pawn when ue5 is not yet out
// // @deprecated used before ue5 
// void ABird::MoveForward(float Value)
// {
// 	if (Controller && Value != 0.f) {
// 		FVector Forward = GetActorForwardVector();
// 		// AddMovementInput only care if Value is +ve or -ve or nth 
// 		AddMovementInput(Forward, Value);
// 	}
// }
// // @deprecated used before ue5
// void ABird::Turn(float Value) {
// 	AddControllerYawInput(Value);
// }

// // @deprecated used before ue5
// void ABird::LookUp(float Value) {
// 	AddControllerPitchInput(Value);
// }

void ABird::Move(const FInputActionValue &Value) {
        const FVector2D MoveValue = Value.Get<FVector2D>();
	if (GetController()) {
		if (MoveValue.X != 0.f) {
			FVector Forward = GetActorForwardVector();
			// AddMovementInput only care if Value is +ve or -ve or nth 
			AddMovementInput(Forward, MoveValue.X);
		}	
		if (MoveValue.Y != 0.f) {
			FVector Right = GetActorRightVector();
			AddMovementInput(Right, MoveValue.Y);
		}
	}
	
}

void ABird::Look(const FInputActionValue &Value) {
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (GetController()) {
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}

}
void ABird::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

// Called to bind functionality to input
void ABird::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	/// Bind inputs with actions & functions defined in cpp file

	// before ue5: use axis mapping
	// bind callback to axis mapping set in project settings in unreal
	// e.g. W keyboard key -> MoveForward AxisName

	/// @param 1 FName or TEXT("MoveForward")
	/// @param 2 this -> pointer of this Bird object
	/// @param 3 pass function ABird::MoveForward by passing the address
	// PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ABird::MoveForward);
	// PlayerInputComponent->BindAxis(FName("Turn"), this, &ABird::Turn);
	// PlayerInputComponent->BindAxis(FName("LookUp"), this, &ABird::LookUp);

	// With EnhancedInput:
	// CashChecked will crash the game if cast failed
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABird::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABird::Look);
	}
 }

