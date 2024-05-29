// Fill out your copyright notice in the Description page of Project Settings.


#include "Slash/Public/Items.h"
#include "Slash/DebugMacros.h"


// Sets default values
AItems::AItems()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AItems::BeginPlay()
{
	Super::BeginPlay();

	/// Add log and on screen message
	// UE_LOG(LogTemp, Warning, TEXT("Begin Play Called"));
	
	// if (GEngine)
	// {
	// 	GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Cyan, FString("Item OnScreen message"));
	// }
}

// Called every frame
void AItems::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Move in units of cm/s
	float MovementRate = 50.f;
	float RotationRate = 45.f;

	// This calculation makes sure that the actor moves at the same rate / s , no matter of all frame rate (across all devices)
	// MovmentRate (cm/s) * DeltaTime (s/frame) => cm/frame
	AddActorWorldOffset(FVector(MovementRate * DeltaTime, 0.f, 0.f));
	AddActorWorldRotation(FRotator(0.f, RotationRate * DeltaTime, 0.f));
	
	DRAW_SPHERE_SingleFrame(GetActorLocation());
	DRAW_VECTOR_SingleFrame(GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 100.f);
}

