#include "AegisWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"

AAegisWeapon::AAegisWeapon()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create weapon mesh component
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

    // Default weapon values
    WeaponRange = 10000.f;  // 100 meters
    WeaponDamage = 20.f;
}

void AAegisWeapon::Fire()
{
    FHitResult Hit;
    FVector ShotDirection;

    // Perform line trace to see what we hit
    bool bHit = PerformLineTrace(Hit, ShotDirection);

    if (bHit)
    {
        // We hit something!
        AActor* HitActor = Hit.GetActor();

        if (HitActor)
        {
            // Apply damage to whatever we hit
            UGameplayStatics::ApplyPointDamage(
                HitActor,
                WeaponDamage,
                ShotDirection,
                Hit,
                GetOwner()->GetInstigatorController(),
                this,
                UDamageType::StaticClass()
            );
        }

        // Spawn impact particle effect at hit location
        if (ImpactEffect)
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                ImpactEffect,
                Hit.ImpactPoint,
                Hit.ImpactNormal.Rotation()
            );
        }

        // Draw debug line to visualize the shot (red line for 2 seconds)
        DrawDebugLine(
            GetWorld(),
            Hit.TraceStart,
            Hit.TraceEnd,
            FColor::Red,
            false,
            2.0f,
            0,
            2.0f
        );
    }

    // Play fire sound
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    }

    // Spawn muzzle flash at weapon
    if (MuzzleFlash && WeaponMesh)
    {
        UGameplayStatics::SpawnEmitterAttached(
            MuzzleFlash,
            WeaponMesh,
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget
        );
    }
}

bool AAegisWeapon::PerformLineTrace(FHitResult& OutHit, FVector& OutShotDirection)
{
    // Get the player controller's view point (where the camera is looking)
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        return false;
    }

    AController* OwnerController = OwnerPawn->GetController();
    if (!OwnerController)
    {
        return false;
    }

    // Get camera location and rotation
    FVector CameraLocation;
    FRotator CameraRotation;
    OwnerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    // Calculate shot direction from camera
    OutShotDirection = CameraRotation.Vector();

    // Calculate end point of line trace
    FVector TraceEnd = CameraLocation + (OutShotDirection * WeaponRange);

    // Set up trace parameters
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);           // Ignore the weapon itself
    QueryParams.AddIgnoredActor(GetOwner());     // Ignore the player
    QueryParams.bTraceComplex = true;            // Use complex collision
    QueryParams.bReturnPhysicalMaterial = true;  // Get material info

    // Perform the line trace
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHit,
        CameraLocation,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    return bHit;
}