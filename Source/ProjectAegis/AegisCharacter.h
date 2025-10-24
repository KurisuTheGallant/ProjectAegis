#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AegisCharacter.generated.h"

// Forward declarations
class AAegisWeapon;
class UCameraComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class PROJECTAEGIS_API AAegisCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Constructor
    AAegisCharacter();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    // Camera component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FirstPersonCamera;

    // Current weapon
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    AAegisWeapon* CurrentWeapon;

    // Weapon class to spawn
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<AAegisWeapon> WeaponClass;

    // Movement functions
    void MoveForward(float Value);
    void MoveRight(float Value);

    // Fire weapon function
    void FireWeapon();

    // === ENERGY STEAL SYSTEM ===

    // Base movement speed (default)
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float BaseMovementSpeed;

    // Speed buff percentage (20% = 0.2)
    UPROPERTY(EditDefaultsOnly, Category = "Energy Steal")
    float SpeedBuffPercentage;

    // Buff duration in seconds
    UPROPERTY(EditDefaultsOnly, Category = "Energy Steal")
    float BuffDuration;

    // Current buff active?
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Energy Steal")
    bool bIsBuffActive;

    // Timer handle for buff duration
    FTimerHandle BuffTimerHandle;

    // === VISUAL & AUDIO FEEDBACK ===

    // Particle effect when getting a kill (energy siphon)
    UPROPERTY(EditDefaultsOnly, Category = "Energy Steal|Effects")
    UParticleSystem* KillParticleEffect;

    // Particle effect for active buff (aura around player)
    UPROPERTY(EditDefaultsOnly, Category = "Energy Steal|Effects")
    UParticleSystem* BuffAuraEffect;

    // Sound when getting energy steal
    UPROPERTY(EditDefaultsOnly, Category = "Energy Steal|Effects")
    USoundBase* EnergySiphonSound;

    // Sound when buff is active (optional looping sound)
    UPROPERTY(EditDefaultsOnly, Category = "Energy Steal|Effects")
    USoundBase* BuffActiveSound;

    // Stored particle component for buff aura (so we can destroy it)
    UPROPERTY()
    class UParticleSystemComponent* ActiveBuffAuraComponent;

    // Apply speed buff
    UFUNCTION()
    void ApplySpeedBuff();

    // Remove speed buff
    UFUNCTION()
    void RemoveSpeedBuff();

    // Play visual and audio feedback
    void PlayEnergyStealFeedback(FVector KillLocation);

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Called when this character gets a kill
    UFUNCTION(BlueprintCallable, Category = "Energy Steal")
    void OnKillEnemy(FVector KillLocation);

    // Get current speed
    UFUNCTION(BlueprintPure, Category = "Energy Steal")
    float GetCurrentSpeed() const;

    // Check if buff is active
    UFUNCTION(BlueprintPure, Category = "Energy Steal")
    bool IsBuffActive() const { return bIsBuffActive; }
};