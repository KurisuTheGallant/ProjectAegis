#include "AegisGrenade.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

AAegisGrenade::AAegisGrenade()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    SetReplicateMovement(true);

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionComponent->InitSphereRadius(15.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("BlockAll"));
    RootComponent = CollisionComponent;

    GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
    GrenadeMesh->SetupAttachment(RootComponent);
    GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
    MovementComponent->UpdatedComponent = CollisionComponent;
    MovementComponent->InitialSpeed = 1500.f;
    MovementComponent->MaxSpeed = 1500.f;
    MovementComponent->bRotationFollowsVelocity = false;
    MovementComponent->bShouldBounce = true;
    MovementComponent->Bounciness = 0.4f;
    MovementComponent->ProjectileGravityScale = 1.0f;

    ExplosionDamage = 80.f;
    ExplosionRadius = 500.f;
    FuseTime = 2.0f;
}

void AAegisGrenade::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("Grenade spawned! Will explode in %.1f seconds"), FuseTime);

    GetWorldTimerManager().SetTimer(
        ExplosionTimerHandle,
        this,
        &AAegisGrenade::Explode,
        FuseTime,
        false
    );
}

void AAegisGrenade::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAegisGrenade::ThrowGrenade(const FVector& ThrowDirection, float ThrowSpeed)
{
    if (MovementComponent)
    {
        MovementComponent->Velocity = ThrowDirection * ThrowSpeed;
        UE_LOG(LogTemp, Warning, TEXT("Grenade thrown! Velocity: %s"), *MovementComponent->Velocity.ToString());
    }
}

void AAegisGrenade::Explode()
{
    UE_LOG(LogTemp, Warning, TEXT("GRENADE EXPLODING at %s!"), *GetActorLocation().ToString());

    FVector ExplosionLocation = GetActorLocation();

    UGameplayStatics::ApplyRadialDamage(
        this,
        ExplosionDamage,
        ExplosionLocation,
        ExplosionRadius,
        UDamageType::StaticClass(),
        TArray<AActor*>(),
        this,
        GetInstigatorController(),
        true
    );

    if (ExplosionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ExplosionEffect,
            ExplosionLocation,
            FRotator::ZeroRotator,
            FVector(3.0f, 3.0f, 3.0f)
        );
    }

    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, ExplosionLocation);
    }

    DrawDebugSphere(
        GetWorld(),
        ExplosionLocation,
        ExplosionRadius,
        32,
        FColor::Red,
        false,
        3.0f,
        0,
        5.0f
    );

    Destroy();
}