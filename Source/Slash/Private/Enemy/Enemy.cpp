// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "AIController.h"
#include "Components/SkeletalMeshComponent.h" 
#include "Components/CapsuleComponent.h" 
#include "Components/AttributeComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HUD/HealthBarComponent.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// update collision so that it will allow weapon hitting
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SetPeripheralVisionAngle(60.f);
	PawnSensing->SightRadius = 1000.f;
	PawnSensing->bOnlySensePlayers = false; 
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyState > EEnemyState::EES_Patrolling) {
		CheckCombatTarget();
	} else {
		CheckPatrolTarget();
	}
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(true);
	}

	if (Attributes && Attributes->IsAlive()) {
		DirectionalHitReact(ImpactPoint);
	} else {
		Die();
	}

	// play hit sound
    if (HitSound) {
        UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			ImpactPoint
		);
    }

	// blood hit particles effect
	if (HitParticles) {
		UGameplayStatics::SpawnEmitterAtLocation(
			this,
			HitParticles,
			ImpactPoint
		);
	}

}

// determine which direction attack is from
// and play the corresponding animation
void AEnemy::DirectionalHitReact(const FVector &ImpactPoint)
{
    
	const FVector Forward = GetActorForwardVector();
	// Lower impact point to the ActorLocation so that it parallel to the ground
	// because we only care about the angle along XY axis
	const FVector ImpactPointLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	// GetSafeNormal normalise within parenthesis without worrying the length is 0
	const FVector ToHit = (ImpactPoint - GetActorLocation()).GetSafeNormal();

	/*
	* Get Angle between Forward Vector to ToHit Vector
	* If -45 < angle < 45, actor got hit from front
	* Else if 45 < angle < 135, actor got hit from left
	* Else if 135 < angle < -135, actor got hit from back
	* Else if -135 < angle < -45, actor got hit from right
	*/

	// Forward . ToHit = |Forward| |ToHit| cos(theta)
	// |Forward| == |ToHit| == 1 (due to normalised)
	// i.e. Forward . ToHit = cos(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	// Take inverse cosine (arc-cosine) for theta
	const double ThetaInRadian = FMath::Acos(CosTheta);
	// convert radians to degrees
	// Degrees always > 0 for both left and right of the actor
	double Degrees = FMath::RadiansToDegrees(ThetaInRadian);

	// Left hand rule
	// If cross product points down, Theta should be negative
	// cross product < 0, ToHit is at the left hand side of the actor
	// else > 0, ToHit is at the right hand side of the actor
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0) {
		Degrees *= -1.f;
	}

	/** Debug
	/// Alternatively, use GetActorRightVector() or so with DotProduct to determine the direction
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + CrossProduct * 100, 5.f, FColor::Blue, 5.f);

	if (GEngine){
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Green, FString::Printf(TEXT("Theta: %f"), Degrees));
	}


	// forward vector and toHit vector
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + Forward * 60, 5.f, FColor::Red, 5.f);
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + ToHit, 5.f, FColor::Green, 5.f);
	 */

	FName SectionName = FName("FromBack");
	if (Degrees >= -45.f && Degrees < 45.f) {
		SectionName = FName("FromFront");
	} else if (Degrees >= -135.f && Degrees < -45.f) {
		SectionName = FName("FromLeft");
	} else if (Degrees >= 45.f && Degrees < 135.f) {
		SectionName = FName("FromRight");
	}
	// play animation montage
	PlayHitReactMontage(SectionName);
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{
	if (Attributes && HealthBarWidget) {
		Attributes->ReceiveDamage(DamageAmount);
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}

	CombatTarget = EventInstigator->GetPawn();
	ChaseTarget(CombatTarget);
	return DamageAmount;
}
void AEnemy::PlayHitReactMontage(const FName &SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage) {
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}
void AEnemy::PlayDeathMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage) {
		AnimInstance->Montage_Play(DeathMontage);

		const int32 Selection = FMath::RandRange(0, 4);
		FName SectionName = FName();
		switch (Selection) {
			case 0:
				SectionName = "Death1";
				DeathPose = EDeathPose::EDP_Death1;
				break;
			case 1:
				SectionName = "Death2";
				DeathPose = EDeathPose::EDP_Death2;
				break;
			case 2:
				SectionName = "Death3";
				DeathPose = EDeathPose::EDP_Death3;
				break;
			case 3:
				SectionName = "Death4";
				DeathPose = EDeathPose::EDP_Death4;
				break;
			case 4:
				SectionName = "Death5";
				DeathPose = EDeathPose::EDP_Death5;
				break;
		}
		AnimInstance->Montage_JumpToSection(SectionName, DeathMontage);
	}
}
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(false);
	}

	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);

	if (PawnSensing) {
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}
}

void AEnemy::Die() {
	// Play death montage
	PlayDeathMontage();

	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(false);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetLifeSpan(3.f);
}

bool AEnemy::InTargetRange(AActor *Target, double Radius)
{
	if (Target == nullptr) return false;
	const float DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

void AEnemy::ChaseTarget(AActor *Target)
{
	EnemyState = EEnemyState::EES_Chasing;
	GetWorldTimerManager().ClearTimer(PatrolTimer);
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	MoveToTarget(Target);
}

void AEnemy::PatrolTimerFinished() 
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::MoveToTarget(AActor* Target) {
    if (EnemyController == nullptr || Target == nullptr) return;
	FAIMoveRequest MoveReq;
	MoveReq.SetGoalActor(Target);
	MoveReq.SetAcceptanceRadius(15.f);
	EnemyController->MoveTo(MoveReq);
}

AActor *AEnemy::ChoosePatrolTarget()
{
    TArray<AActor*> ValidTargets;

	for (auto Target : PatrolTargets) {
		if (Target != PatrolTarget) {
			ValidTargets.AddUnique(Target);
		}
	}
	if (ValidTargets.Num() > 0) {
		const int32 TargetSelection = FMath::RandRange(0, ValidTargets.Num() - 1);
		return ValidTargets[TargetSelection];
	}
	
	return nullptr;
}

void AEnemy::CheckCombatTarget() {
	if (!InTargetRange(CombatTarget, CombatRadius)) {
		// outside combat radius, lose interest

		CombatTarget = nullptr;

		if (HealthBarWidget) {
			HealthBarWidget->SetVisibility(false);
		}

		EnemyState = EEnemyState::EES_Patrolling;
		GetCharacterMovement()->MaxWalkSpeed = 125.f;
		MoveToTarget(PatrolTarget);
	} else if (!InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Chasing) {
		// outside attack range, chase character

		ChaseTarget(CombatTarget);
	
	} else if (InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Attacking) {
		// inside attack range, attack character
		EnemyState = EEnemyState::EES_Attacking;
		// TODO: attack montage
	}
}

void AEnemy::CheckPatrolTarget() {
	if (InTargetRange(PatrolTarget, PatrolRadius)) {
		PatrolTarget = ChoosePatrolTarget();
		const float WaitTime = FMath::RandRange(WaitMin, WaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

void AEnemy::PawnSeen(APawn *SeenPawn)
{
	if (EnemyState == EEnemyState::EES_Chasing) return;
	if (SeenPawn->ActorHasTag(FName("SlashCharacter"))) {
		CombatTarget = SeenPawn;
		if (EnemyState != EEnemyState::EES_Attacking) {
			EnemyState = EEnemyState::EES_Chasing;
			// 		ChaseTarget(CombatTarget);
		}
	}
}
