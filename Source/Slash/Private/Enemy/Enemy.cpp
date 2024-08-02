// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h" 
#include "Components/CapsuleComponent.h" 
#include "Slash/DebugMacros.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"

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
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	// DRAW_SPHERE_COLOR(ImpactPoint, FColor::Orange);
	DirectionalHitReact(ImpactPoint);

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
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
}
