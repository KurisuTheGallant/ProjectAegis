#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AegisWeapon.generated.h"

UCLASS()
class PROJECTAEGIS_API AAegisWeapon : public AActor
{
    GENERATED_BODY()

public:
    // Constructor
    AAegisWeapon();

    // Fire the weapon
    void Fire();

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    // Weapon mesh component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkeletalMeshComponent* WeaponMesh;

    // Weapon stats
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    float WeaponRange;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    float WeaponDamage;

    // Visual effects
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    class UParticleSystem* ImpactEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    class UParticleSystem* MuzzleFlash;

    // Sound
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
    class USoundBase* FireSound;

    // Multicast RPC to show weapon fire effects to all clients
    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayFireEffects(FVector TraceStart, FVector TraceEnd, FVector ImpactPoint, bool bHitSomething);

private:
    // Perform line trace for shooting
    bool PerformLineTrace(FHitResult& OutHit, FVector& OutShotDirection);
};