#include "AegisCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAegisCharacter::AAegisCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create the first-person camera
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f)); // Eye height
    FirstPersonCamera->bUsePawnControlRotation = true;

    // Set default movement values
    GetCharacterMovement()->MaxWalkSpeed = 500.0f; // Standard movement speed
}

void AAegisCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void AAegisCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAegisCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind movement
    PlayerInputComponent->BindAxis("MoveForward", this, &AAegisCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AAegisCharacter::MoveRight);

    // Bind looking
    PlayerInputComponent->BindAxis("LookUp", this, &AAegisCharacter::LookUp);
    PlayerInputComponent->BindAxis("Turn", this, &AAegisCharacter::Turn);

    // Bind jumping
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
}

void AAegisCharacter::MoveForward(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorForwardVector(), Value);
    }
}

void AAegisCharacter::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorRightVector(), Value);
    }
}

void AAegisCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void AAegisCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}