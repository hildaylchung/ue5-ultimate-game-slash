// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	/* <AActor> */
	virtual void Tick(float DeltaTime) override;
	/* </AActor> */

protected:
	/* <AActor> */
	virtual void BeginPlay() override;
	/* </AActor> */

	/* <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/* </IHitInterface> */

	/* Combat related */
	virtual bool CanAttack();
	virtual void Attack();
	virtual void Die();
	bool IsAlive();
	void DirectionalHitReact(const FVector& ImpactPoint);
	virtual void HandleDamage(float DamageAmount);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	void DisableCapsuleCollision();

	/* Montages */
	void PlayHitReactMontage(const FName& SectionName);
	void PlayEquipMontage(const FName &SectionName);
	virtual int32 PlayAttackMontage();
	virtual int32 PlayDeathMontage();
	void StopAttackMontage();

	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();

	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY(BlueprintReadWrite, Category = Combat)
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere, Category = Combat)
	double WarpTargetDistance = 75.f;
	
private:
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	int32 PlayRandomMontageSection(UAnimMontage* Montage);
	
	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	UParticleSystem* HitParticles;

	/** 
	 * Animation Montages
	 */
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* HitReactMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* EquipMontage;
};
