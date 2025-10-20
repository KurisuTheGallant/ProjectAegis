#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AegisCharacter.generated.h"

UCLASS()
class PROJECTAEGIS_API AAegisCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AAegisCharacter();

protected:
    virtual void BeginPlay() override;

    // Camera component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* FirstPersonCamera;

    // Movement functions
    void MoveForward(float Value);
    void MoveRight(float Value);
    void LookUp(float Value);
    void Turn(float Value);

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};