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

	UWorld* World = GetWorld();

	// set location and rotation
	// SetActorLocation(FVector(0.f, 0.f, 50.f));
	// SetActorRotation(FRotator(0.f, 45.f, 0.f));

	FVector Location = GetActorLocation();
	FVector Forward = GetActorForwardVector();
	
	// Draw debug sphere
	DRAW_SPHERE(Location);

	// // Draw debug line
	// DRAW_LINE(Location, Location + Forward * 100.f);

	// // Draw Debug Point
	// DRAW_POINT(Location + Forward * 100.f);

	// Draw vector macro including both
	DRAW_VECTOR(Location, Location + Forward * 100.f);

	// Challenge: Draw other debug items
	DRAW_CAPSULE(Location + Forward * 100.f);
}

// Called every frame
void AItems::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/// ADD log and on screen message
	// UE_LOG(LogTemp, Warning, TEXT("DeltaTime: %f"), DeltaTime);

	// if (GEngine)
	// {
	// 	FString Name = GetName();
	// 	FString Message = FString::Printf(TEXT("Item Name: %s"), *Name);
	// 	GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Cyan, Message);
	// }
}

