// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"
#include "Weapons/Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h" 
#include "Kismet/GameplayStatics.h"

#include "Slash/DebugMacros.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::Attack()
{
}
void ABaseCharacter::Die() {}

int32 ABaseCharacter::PlayAttackMontage() {
	return PlayRandomMontageSection(AttackMontage);
}

int32 ABaseCharacter::PlayDeathMontage()
{
	return PlayRandomMontageSection(DeathMontage);
}

void ABaseCharacter::PlayHitReactMontage(const FName &SectionName) {
	PlayMontageSection(HitReactMontage, SectionName);
}

// determine which direction attack is from
// and play the corresponding animation
void ABaseCharacter::DirectionalHitReact(const FVector &ImpactPoint)
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
void ABaseCharacter::PlayHitSound(const FVector &ImpactPoint)
{
	// play hit sound
    if (HitSound) {
        UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			ImpactPoint
		);
    }
}
void ABaseCharacter::SpawnHitParticles(const FVector &ImpactPoint)
{
	if (HitParticles) {
		UGameplayStatics::SpawnEmitterAtLocation(
			this,
			HitParticles,
			ImpactPoint
		);
	}
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attributes) {
		Attributes->ReceiveDamage(DamageAmount);
	}
}
void ABaseCharacter::PlayMontageSection(UAnimMontage *Montage, const FName &SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage) {
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}
int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage *Montage)
{
	const int32 NumMontageSections = Montage->GetNumSections();  
	if (NumMontageSections <= 0) return -1; 
	const int32 Selection = FMath::RandRange(0, NumMontageSections -1);
	const FName Section = Montage->GetSectionName(Selection);
	PlayMontageSection(Montage, Section);
    return Selection;
}
bool ABaseCharacter::CanAttack()
{
    return false;
}
bool ABaseCharacter::IsAlive()
{
    return Attributes && Attributes->IsAlive();
}
void ABaseCharacter::AttackEnd() {}
void ABaseCharacter::SetWeaponCollisionEnabled(
    ECollisionEnabled::Type CollisionEnabled) {
  if (EquippedWeapon && EquippedWeapon->GetWeaponBox()) {
    EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
    EquippedWeapon->ResetIgnoreActors();
  }
}
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
 
void ABaseCharacter::DisableCapsuleCollision()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}