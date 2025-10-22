#include "DummyTarget.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
ADummyTarget::ADummyTarget()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//set Default Health 
	MaxHealth = 100.f;
	CurrentHealth = MaxHealth;
	bIsDead = false;

	//disable Movement for Dummy Target
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetMovementMode(MOVE_None);

	//set collisions to respond to all channels
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

// Called when the game starts or when spawned
void ADummyTarget::BeginPlay()
{
	Super::BeginPlay();
	
	//Reser health on Spawn
	CurrentHealth = MaxHealth;
	bIsDead = false;
}

// Called every frame
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
        Die();
    }

    return ActualDamage;
}

void ADummyTarget::Die()
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;

    UE_LOG(LogTemp, Warning, TEXT("DummyTarget died!"));

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Enable ragdoll physics
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

    // Destroy after 5 seconds
    SetLifeSpan(5.0f);
}

