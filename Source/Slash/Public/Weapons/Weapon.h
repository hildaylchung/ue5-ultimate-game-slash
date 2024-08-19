// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

class USoundBase;
class UBoxComponent;

/**
 * 
 */
UCLASS()
class SLASH_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapon();

	/* Getter functions */
	FORCEINLINE bool GetIsTwoHanded() { return bIsTwoHanded; };
	FORCEINLINE UBoxComponent* GetWeaponBox() { return WeaponBox; };

	void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator);
	void AttachMeshToSocket(USceneComponent *InParent, const FName& InSocketName);
	void ResetIgnoreActors();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsTwoHanded;

private:
	void PlayEquipSound();
	void DisableSphereCollision();
	void DisactivateEmbers();
	void BoxTrace(FHitResult& BoxHit);
	void ExecuteGetHit(FHitResult& BoxHit);
	bool ActorIsSameType(AActor* OtherActor);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FVector BoxTraceExtent = FVector(5.f);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bShowBoxDebug = false;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Damage = 20;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	USoundBase* EquipSound;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UBoxComponent* WeaponBox;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceStart;
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceEnd;

	TArray<AActor*> IgnoreActors;
};
