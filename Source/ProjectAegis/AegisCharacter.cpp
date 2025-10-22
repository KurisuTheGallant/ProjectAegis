#include "AegisCharacter.h"
#include "AegisWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Constructor
AAegisCharacter::AAegisCharacter()
{
    // Set this character to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Create a camera component
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f)); // Position the camera
    FirstPersonCamera->bUsePawnControlRotation = true;

    // Configure character movement
    GetCharacterMovement()->MaxWalkSpeed = 600.f;
    GetCharacterMovement()->JumpZVelocity = 600.f;
}

// Called when the game starts or when spawned
void AAegisCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Spawn and attach weapon
    if (WeaponClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();

        CurrentWeapon = GetWorld()->SpawnActor<AAegisWeapon>(WeaponClass, SpawnParams);

        if (CurrentWeapon && FirstPersonCamera)
        {
            // Attach weapon to camera
            CurrentWeapon->AttachToComponent(
                FirstPersonCamera,
                FAttachmentTransformRules::SnapToTargetIncludingScale
            );

            // Position weapon in front of camera
            CurrentWeapon->SetActorRelativeLocation(FVector(30.f, 10.f, -10.f));
        }
    }
}

// Called every frame
void AAegisCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AAegisCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Movement bindings
    PlayerInputComponent->BindAxis("MoveForward", this, &AAegisCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AAegisCharacter::MoveRight);

    // Look bindings
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

    // Jump binding
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    // Fire weapon binding
    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AAegisCharacter::FireWeapon);
}

// Move forward/backward
void AAegisCharacter::MoveForward(float Value)
{
    if (Value != 0.0f)
    {
        // Find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // Get forward vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

// Move right/left
void AAegisCharacter::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        // Find out which way is right
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // Get right vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

// Fire weapon
void AAegisCharacter::FireWeapon()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->Fire();
    }
}