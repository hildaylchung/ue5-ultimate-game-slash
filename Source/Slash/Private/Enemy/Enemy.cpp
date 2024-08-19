// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "AIController.h"
#include "Components/AttributeComponent.h"
#include "Components/SkeletalMeshComponent.h" 
#include "Weapons/Weapon.h"
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

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensingComponent->SetPeripheralVisionAngle(60.f);
	PawnSensingComponent->SightRadius = 1000.f;
	PawnSensingComponent->bOnlySensePlayers = false; 
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;
	if (EnemyState > EEnemyState::EES_Patrolling) {
		CheckCombatTarget();
	} else {
		CheckPatrolTarget();
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();
	ChaseTarget(CombatTarget); 
	return DamageAmount;
}

void AEnemy::Destroyed()
{
	if (EquippedWeapon) {
		EquippedWeapon->Destroy();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	ShowHealthBar();

	if (IsAlive()) {
		DirectionalHitReact(ImpactPoint);
	} else {
		Die();
	}

	PlayHitSound(ImpactPoint);	
	SpawnHitParticles(ImpactPoint);	
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (PawnSensingComponent) PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	InitializeEnemy();
	Tags.Add(FName("Enemy"));
}

void AEnemy::Die()
{
	EnemyState = EEnemyState::EES_Dead;
	PlayDeathMontage();	
	ClearAttackTimer();
	HideHealthBar();
	DisableCapsuleCollision();
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetLifeSpan(DeathLifeSpan);
}

void AEnemy::Attack() {
	EnemyState = EEnemyState::EES_Engaged;
	Super::Attack();
	PlayAttackMontage();
}

bool AEnemy::CanAttack()
{
    return !IsDead() && 
		IsInsideAttackRadius() && 
		!IsAttacking() && 
		!IsEngaged();
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

void AEnemy::HandleDamage(float DamageAmount) {
	Super::HandleDamage(DamageAmount);
	if (HealthBarWidget) {
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

int32 AEnemy::PlayDeathMontage()
{
	const int32 Selection = Super::PlayDeathMontage();
	TEnumAsByte<EDeathPose> Pose(Selection);
	// Alternative solutions:
	// EDeathPose Pose = static_cast<EDeathPose>(Selection);

	if (Pose < EDeathPose::EDP_MAX) {
		DeathPose = Pose;
	}

	return Selection;
}

void AEnemy::InitializeEnemy()
{
	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);
	HideHealthBar();
	SpawnDefaultWeapon();
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(false);
    }
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(true);
	}
}

bool AEnemy::InTargetRange(AActor *Target, double Radius)
{
	if (Target == nullptr) return false;
	const float DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && WeaponClass) {
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

void AEnemy::CheckPatrolTarget() {
	if (InTargetRange(PatrolTarget, PatrolRadius)) {
		PatrolTarget = ChoosePatrolTarget();
		const float WaitTime = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
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

void AEnemy::MoveToTarget(AActor* Target) {
    if (EnemyController == nullptr || Target == nullptr) return;
	FAIMoveRequest MoveReq;
	MoveReq.SetGoalActor(Target);
	MoveReq.SetAcceptanceRadius(50.f);
	EnemyController->MoveTo(MoveReq);
}

void AEnemy::ChaseTarget(AActor *Target)
{
	EnemyState = EEnemyState::EES_Chasing;
	ClearPatrolTimer();
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(Target);
}

void AEnemy::StartPatrolling() {
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemy::CheckCombatTarget()
{
    if (IsOutsideCombatRadius()) {
		ClearAttackTimer();
		LoseInterest();
		if (!IsEngaged()) StartPatrolling();
	} else if (IsOutsideAttackRadius() && !IsChasing()) {
		ClearAttackTimer();
		if (!IsEngaged()) ChaseTarget(CombatTarget);
	} else if (CanAttack()) {
		StartAttackTimer();
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	HideHealthBar();
}

void AEnemy::PatrolTimerFinished() {
	MoveToTarget(PatrolTarget);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ClearPatrolTimer()
{
    GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::StartAttackTimer() {
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer() {
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

bool AEnemy::IsOutsideCombatRadius()
{
    return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
    return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius()
{
    return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsDead()
{
    return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsChasing()
{
    return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking()
{
    return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsEngaged()
{
    return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::PawnSeen(APawn *SeenPawn)
{
	const bool bShouldChaseTarget = 
		EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Chasing && 
		EnemyState < EEnemyState::EES_Attacking &&
		SeenPawn->ActorHasTag(FName("EngageableTarget"));

	if (bShouldChaseTarget) {
		CombatTarget = SeenPawn;
		ChaseTarget(CombatTarget);
	}
}

