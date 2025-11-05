#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AegisGrenade.generated.h"

UCLASS()
class PROJECTAEGIS_API AAegisGrenade : public AActor
{
    GENERATED_BODY()

public:
    AAegisGrenade();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* GrenadeMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UProjectileMovementComponent* MovementComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Grenade")
    float ExplosionDamage;

    UPROPERTY(EditDefaultsOnly, Category = "Grenade")
    float ExplosionRadius;

    UPROPERTY(EditDefaultsOnly, Category = "Grenade")
    float FuseTime;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    class UParticleSystem* ExplosionEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    class USoundBase* ExplosionSound;

    FTimerHandle ExplosionTimerHandle;

    UFUNCTION()
    void Explode();

public:
    virtual void Tick(float DeltaTime) override;

    void ThrowGrenade(const FVector& ThrowDirection, float ThrowSpeed);
};