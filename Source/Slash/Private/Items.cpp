// Fill out your copyright notice in the Description page of Project Settings.


#include "Slash/Public/Items.h"
#include "Slash/DebugMacros.h"
#include "Items.h"


// Sets default values
AItems::AItems() : Amplitude(0.25f), TimeConstant(5.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Another different way to init private variables
	// Amplitude = 0.25f;
	// TimeConstant = 5.f;
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

float AItems::TransformedSin()
{
    return Amplitude * FMath::Sin(RunningTime * TimeConstant); // period of wave = 2*pi/TimeConstant
}

float AItems::TransformedCos()
{
    return Amplitude * FMath::Cos(RunningTime * TimeConstant); // period of wave = 2*pi/TimeConstant
}

 // Called every frame
void AItems::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	// float DeltaZ = TransformedSin(RunningTime); // period of wave = 2*pi/TimeConstant
	// AddActorWorldOffset(FVector(0.f, 0.f, DeltaZ));

	DRAW_SPHERE_SingleFrame(GetActorLocation());
	DRAW_VECTOR_SingleFrame(GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 100.f);

	FVector AvgVector = Avg<FVector>(GetActorLocation(), FVector::ZeroVector);
	DRAW_POINT_SingleFrame(AvgVector);

	// this will fail coz FRotator does not have divider or + operator implementation
	// FRotator AvgRotator = Avg<FRotator>(GetActorRotation(), FRotator::ZeroRotator);
}
