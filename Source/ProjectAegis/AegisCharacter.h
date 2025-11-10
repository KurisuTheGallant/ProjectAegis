#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AegisCharacter.generated.h"

// Forward declarations
class AAegisWeapon;
class UCameraComponent;
class UParticleSystem;
class USoundBase;
class AAegisGrenade;

UCLASS()
class PROJECTAEGIS_API AAegisCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Constructor
    AAegisCharacter();

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

    // === ABILITY SYSTEM ===

    // Grenade class to spawn
    UPROPERTY(EditDefaultsOnly, Category = "Abilities")
    TSubclassOf<AAegisGrenade> GrenadeClass;

    // Grenade cooldown time
    UPROPERTY(EditDefaultsOnly, Category = "Abilities")
    float GrenadeCooldown;

    // Is grenade on cooldown?
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
    bool bGrenadeOnCooldown;

    // Grenade throw speed
    UPROPERTY(EditDefaultsOnly, Category = "Abilities")
    float GrenadeThrowSpeed;

    // Timer handle for grenade cooldown
    FTimerHandle GrenadeCooldownTimerHandle;

    // Use grenade ability
    void UseGrenadeAbility();

    // Server RPC for grenade
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerUseGrenade();

    // Helper function to spawn grenade
    void SpawnAndThrowGrenade();

    // Reset grenade cooldown
    UFUNCTION()
    void ResetGrenadeCooldown();

    // Movement functions
    void MoveForward(float Value);
    void MoveRight(float Value);

    // Fire weapon function
    void FireWeapon();

    // Server RPC for firing
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerFireWeapon();

    // === HEALTH SYSTEM ===

    // Maximum health
    UPROPERTY(Replicated, EditDefaultsOnly, Category = "Health")
    float MaxHealth;

    // Current health
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    float CurrentHealth;

    // Is player dead?
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    bool bIsDead;

    // Store the last controller who damaged us
    UPROPERTY()
    AController* LastDamageCauser;

    // Respawn delay in seconds
    UPROPERTY(EditDefaultsOnly, Category = "Health")
    float RespawnDelay;

    // Timer handle for respawn
    FTimerHandle RespawnTimerHandle;

    // Death function
    UFUNCTION()
    void Die();

    // Respawn function
    UFUNCTION()
    void Respawn();

    // Multicast RPC for death effects
    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayDeathEffects();

    // Multicast RPC for respawn effects
    UFUNCTION(NetMulticast, Reliable)
    void MulticastRespawnEffects();

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
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Energy Steal")
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

    // Take damage override
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, AActor* DamageCauser) override;

    // Called when this character gets a kill
    UFUNCTION(BlueprintCallable, Category = "Energy Steal")
    void OnKillEnemy(FVector KillLocation);

    // Get current speed
    UFUNCTION(BlueprintPure, Category = "Energy Steal")
    float GetCurrentSpeed() const;

    // Check if buff is active
    UFUNCTION(BlueprintPure, Category = "Energy Steal")
    bool IsBuffActive() const { return bIsBuffActive; }

    // Get health percentage (for UI)
    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercent() const { return CurrentHealth / MaxHealth; }

    // Get current health
    UFUNCTION(BlueprintPure, Category = "Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    // Check if dead
    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsDead() const { return bIsDead; }

    // Check if grenade is on cooldown
    UFUNCTION(BlueprintPure, Category = "Abilities")
    bool IsGrenadeOnCooldown() const { return bGrenadeOnCooldown; }

    // Get grenade cooldown remaining (for UI)
    UFUNCTION(BlueprintPure, Category = "Abilities")
    float GetGrenadeCooldownRemaining() const;
};