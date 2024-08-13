// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class AAIController;
class UPawnSensingComponent;

UCLASS()class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*
	*	Override functions
	*/
	void GetHit_Implementation(const FVector& ImpactPoint) override;
	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/**
	 *  Animation Montages
	 */

	void PlayDeathMontage();


protected:
	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose = EDeathPose::EDP_Alive;

	virtual void BeginPlay() override;

	virtual void Die() override;
	bool InTargetRange(AActor* Target, double Radius); 
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);

private:
	/*
	* States
	*/

	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	/*
	* Components
	*/

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere)
	double CombatRadius = 500.f;
	
	UPROPERTY(EditAnywhere)
	double AttackRadius = 150.f;

	void ChaseTarget(AActor* Target);

	/* 
	* Navigation
	*/

	UPROPERTY()
	AAIController* EnemyController;

	// Current patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditInstanceOnly)
	double PatrolRadius = 200.f;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	float WaitMin = 5.f;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	float WaitMax = 10.f;

	FTimerHandle PatrolTimer;

	void PatrolTimerFinished();


	void CheckCombatTarget();
	void CheckPatrolTarget();

};
