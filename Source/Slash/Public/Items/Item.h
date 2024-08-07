// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"


class USphereComponent;
class UNiagaraComponent;

enum class EItemState : uint8 {
	EIS_Hovering,
	EIS_Equipped
};

UCLASS()
class SLASH_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AItem();
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

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* ItemMesh;

	EItemState ItemState = EItemState::EIS_Hovering;

	// components
	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere)
	UNiagaraComponent* EmbersEffect;

private:
	// meta allow the variable to be exposed in blueprint even it is private
	// AllowPrivateAccess = "true" or true also valid
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float RunningTime;

	// also legal but not of the best way to expose variables in header files
	// float Amplitude = 0.25f;
	// float TimeConstant = 5.f;

};

template <typename T>
inline T AItem::Avg(T First, T Second)
{
    return (First + Second) / 2;
}
