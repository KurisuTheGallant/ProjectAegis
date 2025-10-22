#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AegisCharacter.generated.h"

// Forward declaration
class AAegisWeapon;
class UCameraComponent;

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

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};