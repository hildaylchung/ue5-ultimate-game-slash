// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items.generated.h"

UCLASS()
class SLASH_API AItems : public AActor
{
	GENERATED_BODY()
	
public:	
	AItems();
    virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// Blueprint related uproperty cannot be used in private variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameters")
	float Amplitude;

	// same category with "Sine Parameters"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SineParameters)
	float TimeConstant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationAmount;

	UFUNCTION(BlueprintPure)
	float RotatedSin();

	UFUNCTION(BlueprintPure)
	float TransformedSin();

	UFUNCTION(BlueprintPure)
	float TransformedCos();

	template<typename T>
	T Avg(T First, T Second);

private:
	// meta allow the variable to be exposed in blueprint even it is private
	// AllowPrivateAccess = "true" or true also valid
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float RunningTime;

	// also legal but not of the best way to expose variables in header files
	// float Amplitude = 0.25f;
	// float TimeConstant = 5.f;

	// components
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ItemMesh;
};

template <typename T>
inline T AItems::Avg(T First, T Second)
{
    return (First + Second) / 2;
}
