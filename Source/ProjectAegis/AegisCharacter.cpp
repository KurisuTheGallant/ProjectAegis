#include "AegisCharacter.h"
#include "AegisWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

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
    BaseMovementSpeed = 600.f;
    GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
    GetCharacterMovement()->JumpZVelocity = 600.f;

    // Health system defaults
    MaxHealth = 100.f;
    CurrentHealth = MaxHealth;
    bIsDead = false;
    RespawnDelay = 3.0f;

    // Energy Steal system defaults
    SpeedBuffPercentage = 0.2f;  // 20% speed increase
    BuffDuration = 5.0f;          // 5 seconds
    bIsBuffActive = false;

    // Initialize effect pointers
    ActiveBuffAuraComponent = nullptr;
}

// Called when the game starts or when spawned
void AAegisCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Initialize health
    CurrentHealth = MaxHealth;
    bIsDead = false;

    // Store base speed
    BaseMovementSpeed = GetCharacterMovement()->MaxWalkSpeed;

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
    if (Value != 0.0f && !bIsDead)
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
    if (Value != 0.0f && !bIsDead)
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
    if (CurrentWeapon && !bIsDead)
    {
        CurrentWeapon->Fire();
    }
}

// === HEALTH SYSTEM IMPLEMENTATION ===

float AAegisCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead)
    {
        return 0.0f;
    }

    // Calculate actual damage
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // Reduce health
    CurrentHealth -= ActualDamage;
    CurrentHealth = FMath::Max(CurrentHealth, 0.0f);

    UE_LOG(LogTemp, Warning, TEXT("Player took %.2f damage. Health: %.2f / %.2f"),
        ActualDamage, CurrentHealth, MaxHealth);

    // Check if dead
    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return ActualDamage;
}

void AAegisCharacter::Die()
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;

    UE_LOG(LogTemp, Warning, TEXT("Player died!"));

    // Disable input
    DisableInput(Cast<APlayerController>(GetController()));

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Hide weapon
    if (CurrentWeapon)
    {
        CurrentWeapon->SetActorHiddenInGame(true);
    }

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

    // Remove any active buffs
    RemoveSpeedBuff();

    // Set respawn timer
    GetWorldTimerManager().SetTimer(
        RespawnTimerHandle,
        this,
        &AAegisCharacter::Respawn,
        RespawnDelay,
        false
    );
}

void AAegisCharacter::Respawn()
{
    UE_LOG(LogTemp, Warning, TEXT("Player respawning..."));

    // Find a player start
    APlayerStart* PlayerStart = nullptr;
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        PlayerStart = *It;
        break;
    }

    if (PlayerStart)
    {
        // Teleport to player start
        SetActorLocation(PlayerStart->GetActorLocation());
        SetActorRotation(PlayerStart->GetActorRotation());
    }

    // Reset health
    CurrentHealth = MaxHealth;
    bIsDead = false;

    // Re-enable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetSimulatePhysics(false);
    GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
    GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    // Show weapon
    if (CurrentWeapon)
    {
        CurrentWeapon->SetActorHiddenInGame(false);
    }

    // Re-enable input
    EnableInput(Cast<APlayerController>(GetController()));

    UE_LOG(LogTemp, Warning, TEXT("Player respawned!"));
}

// === ENERGY STEAL SYSTEM IMPLEMENTATION ===

void AAegisCharacter::OnKillEnemy(FVector KillLocation)
{
    if (bIsDead)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("OnKillEnemy called! Applying speed buff..."));

    // Play visual and audio feedback
    PlayEnergyStealFeedback(KillLocation);

    // Apply the speed buff
    ApplySpeedBuff();
}

void AAegisCharacter::ApplySpeedBuff()
{
    if (!GetCharacterMovement() || bIsDead)
    {
        return;
    }

    // Calculate new speed (base speed + 20%)
    float BuffedSpeed = BaseMovementSpeed * (1.0f + SpeedBuffPercentage);
    GetCharacterMovement()->MaxWalkSpeed = BuffedSpeed;

    bIsBuffActive = true;

    UE_LOG(LogTemp, Warning, TEXT("Speed buff applied! Speed: %.2f (was %.2f)"),
        BuffedSpeed, BaseMovementSpeed);

    // Spawn buff aura particle effect (attached to player)
    if (BuffAuraEffect && !ActiveBuffAuraComponent)
    {
        ActiveBuffAuraComponent = UGameplayStatics::SpawnEmitterAttached(
            BuffAuraEffect,
            GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }

    // Play buff active sound (looping if set up that way)
    if (BuffActiveSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BuffActiveSound, GetActorLocation());
    }

    // Clear any existing timer
    GetWorldTimerManager().ClearTimer(BuffTimerHandle);

    // Set timer to remove buff after duration
    GetWorldTimerManager().SetTimer(
        BuffTimerHandle,
        this,
        &AAegisCharacter::RemoveSpeedBuff,
        BuffDuration,
        false
    );
}

void AAegisCharacter::RemoveSpeedBuff()
{
    if (!GetCharacterMovement())
    {
        return;
    }

    // Reset to base speed
    GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
    bIsBuffActive = false;

    UE_LOG(LogTemp, Warning, TEXT("Speed buff removed! Speed back to: %.2f"), BaseMovementSpeed);

    // Destroy buff aura particle
    if (ActiveBuffAuraComponent)
    {
        ActiveBuffAuraComponent->DestroyComponent();
        ActiveBuffAuraComponent = nullptr;
    }
}

void AAegisCharacter::PlayEnergyStealFeedback(FVector KillLocation)
{
    // Spawn particle effect at kill location (energy siphon effect)
    if (KillParticleEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            KillParticleEffect,
            KillLocation,
            FRotator::ZeroRotator,
            FVector(1.5f, 1.5f, 1.5f)  // Slightly larger scale
        );

        UE_LOG(LogTemp, Warning, TEXT("Spawned kill particle effect at kill location!"));
    }

    // Play energy siphon sound
    if (EnergySiphonSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, EnergySiphonSound, GetActorLocation());
        UE_LOG(LogTemp, Warning, TEXT("Played energy siphon sound!"));
    }
}

float AAegisCharacter::GetCurrentSpeed() const
{
    if (GetCharacterMovement())
    {
        return GetCharacterMovement()->MaxWalkSpeed;
    }
    return 0.0f;
}