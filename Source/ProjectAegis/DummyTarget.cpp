#include "DummyTarget.h"
#include "AegisCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ADummyTarget::ADummyTarget()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set default health
    MaxHealth = 100.f;
    CurrentHealth = MaxHealth;
    bIsDead = false;

    // Disable movement for dummy
    GetCharacterMovement()->DisableMovement();
    GetCharacterMovement()->SetMovementMode(MOVE_None);

    // Set collision to respond to all channels
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ADummyTarget::BeginPlay()
{
    Super::BeginPlay();

    // Reset health on spawn
    CurrentHealth = MaxHealth;
    bIsDead = false;
}

void ADummyTarget::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

float ADummyTarget::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    // Don't take damage if already dead
    if (bIsDead)
    {
        return 0.0f;
    }

    // Calculate actual damage
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // Reduce health
    CurrentHealth -= ActualDamage;
    CurrentHealth = FMath::Max(CurrentHealth, 0.0f);

    // Log damage for debugging
    UE_LOG(LogTemp, Warning, TEXT("DummyTarget took %.2f damage. Health: %.2f / %.2f"),
        ActualDamage, CurrentHealth, MaxHealth);

    // Check if dead
    if (CurrentHealth <= 0.0f)
    {
        Die(EventInstigator);
    }

    return ActualDamage;
}

void ADummyTarget::Die(AController* Killer)
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;

    UE_LOG(LogTemp, Warning, TEXT("DummyTarget died!"));

    // Get death location for particle effect
    FVector DeathLocation = GetActorLocation();

    // === ENERGY STEAL: Notify the killer! ===
    if (Killer)
    {
        APawn* KillerPawn = Killer->GetPawn();
        if (KillerPawn)
        {
            AAegisCharacter* AegisChar = Cast<AAegisCharacter>(KillerPawn);
            if (AegisChar)
            {
                // Trigger the Energy Steal mechanic with kill location!
                AegisChar->OnKillEnemy(DeathLocation);
                UE_LOG(LogTemp, Warning, TEXT("Notified AegisCharacter of kill at location!"));
            }
        }
    }

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Enable ragdoll physics
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

    // Destroy after 5 seconds
    SetLifeSpan(5.0f);
}