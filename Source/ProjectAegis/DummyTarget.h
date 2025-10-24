#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DummyTarget.generated.h"

UCLASS()
class PROJECTAEGIS_API ADummyTarget : public ACharacter
{
    GENERATED_BODY()

public:
    // Constructor
    ADummyTarget();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    // Health system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    float CurrentHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    bool bIsDead;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Take damage override
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, AActor* DamageCauser) override;

    // Death function
    UFUNCTION()
    void Die(AController* Killer);

    // Check if dead
    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsDead() const { return bIsDead; }

    // Get health percentage
    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercent() const { return CurrentHealth / MaxHealth; }
};