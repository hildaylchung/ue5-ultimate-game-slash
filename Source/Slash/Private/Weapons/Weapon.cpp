// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h" 
#include "Interfaces/HitInterface.h"
#include "NiagaraComponent.h"


AWeapon::AWeapon() {
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
    WeaponBox->SetupAttachment(GetRootComponent());
    WeaponBox->SetBoxExtent(FVector(1.0f, 0.7f, 40.0f));

    // collision only enabled in certain attack time
    WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
    WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

    BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
    BoxTraceStart->SetupAttachment(GetRootComponent());
    
    BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
    BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void AWeapon::Equip(USceneComponent *InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
    ItemState = EItemState::EIS_Equipped;
    SetOwner(NewOwner);
    SetInstigator(NewInstigator);
    AttachMeshToSocket(InParent, InSocketName);

    DisableSphereCollision();
    PlayEquipSound();
    DisactivateEmbers();
}

void AWeapon::AttachMeshToSocket(USceneComponent *InParent, const FName& InSocketName)
{
    FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
    ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}

void AWeapon::ResetIgnoreActors()
{
    IgnoreActors.Empty();
    if (GetOwner()) {
        IgnoreActors.AddUnique(GetOwner());
    }
}

void AWeapon::BeginPlay() {
    Super::BeginPlay();
    WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
    ResetIgnoreActors();
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult) {
    
    if (ActorIsSameType(OtherActor)) return;
    
    FHitResult BoxHit;
    BoxTrace(BoxHit);
    if (BoxHit.GetActor()) {
        if (ActorIsSameType(BoxHit.GetActor())) return;
        UGameplayStatics::ApplyDamage(
            BoxHit.GetActor(),
            Damage,
            GetInstigator()->GetController(),
            this,
            UDamageType::StaticClass()
        );
        
        ExecuteGetHit(BoxHit);
        CreateFields(BoxHit.ImpactPoint);
    }
}

void AWeapon::PlayEquipSound() {
    if (EquipSound) {
        UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
    }
}

void AWeapon::DisableSphereCollision() {
    if (SphereComponent) {
        SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AWeapon::DisactivateEmbers() {
    if (EmbersEffect) {
        EmbersEffect->Deactivate();
    }
}

void AWeapon::BoxTrace(FHitResult &BoxHit)
{
    const FVector Start = BoxTraceStart-> GetComponentLocation();
    const FVector End = BoxTraceEnd-> GetComponentLocation();
    
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    for (AActor* Actor : IgnoreActors) {
        ActorsToIgnore.AddUnique(Actor);
    }

    // HitResult will be stored in variable BoxHit    
    UKismetSystemLibrary::BoxTraceSingle(
        this,
        Start,
        End,
        BoxTraceExtent,
        BoxTraceStart->GetComponentRotation(),
        ETraceTypeQuery::TraceTypeQuery1,
        false,
        ActorsToIgnore,
        bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
        BoxHit,
        true
    );
    IgnoreActors.AddUnique(BoxHit.GetActor());
}

void AWeapon::ExecuteGetHit(FHitResult& BoxHit) {
    if (IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor())) {
        // blueprint native event
        // call GetHit_Implementation instead of blueprint if available
        HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint);
    }
}

bool AWeapon::ActorIsSameType(AActor *OtherActor)
{
    return GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));
}
