#include "AegisCharacter.h"
#include "AegisWeapon.h"
#include "AegisGrenade.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

AAegisCharacter::AAegisCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Enable replication
    bReplicates = true;
    SetReplicateMovement(true);

    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
    FirstPersonCamera->bUsePawnControlRotation = true;

    BaseMovementSpeed = 600.f;
    GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
    GetCharacterMovement()->JumpZVelocity = 600.f;

    MaxHealth = 100.f;
    CurrentHealth = MaxHealth;
    bIsDead = false;
    RespawnDelay = 3.0f;

    SpeedBuffPercentage = 0.2f;
    BuffDuration = 5.0f;
    bIsBuffActive = false;

    GrenadeCooldown = 15.0f;
    bGrenadeOnCooldown = false;
    GrenadeThrowSpeed = 1500.f;

    ActiveBuffAuraComponent = nullptr;
}

void AAegisCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate health to all clients
    DOREPLIFETIME(AAegisCharacter, CurrentHealth);
    DOREPLIFETIME(AAegisCharacter, MaxHealth);
    DOREPLIFETIME(AAegisCharacter, bIsDead);

    // Replicate speed buff
    DOREPLIFETIME(AAegisCharacter, bIsBuffActive);

    // Replicate grenade cooldown
    DOREPLIFETIME(AAegisCharacter, bGrenadeOnCooldown);
}

void AAegisCharacter::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    bIsDead = false;

    BaseMovementSpeed = GetCharacterMovement()->MaxWalkSpeed;

    // Setup third-person mesh (what other players see)
    if (GetMesh())
    {
        GetMesh()->SetOwnerNoSee(true); // Owner doesn't see their own body
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
    }

    if (WeaponClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();

        CurrentWeapon = GetWorld()->SpawnActor<AAegisWeapon>(WeaponClass, SpawnParams);

        if (CurrentWeapon && FirstPersonCamera)
        {
            CurrentWeapon->AttachToComponent(
                FirstPersonCamera,
                FAttachmentTransformRules::SnapToTargetIncludingScale
            );

            CurrentWeapon->SetActorRelativeLocation(FVector(30.f, 10.f, -10.f));
        }
    }
}

void AAegisCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAegisCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AAegisCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AAegisCharacter::MoveRight);

    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AAegisCharacter::FireWeapon);

    PlayerInputComponent->BindAction("UseGrenade", IE_Pressed, this, &AAegisCharacter::UseGrenadeAbility);
}

void AAegisCharacter::MoveForward(float Value)
{
    if (Value != 0.0f && !bIsDead)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void AAegisCharacter::MoveRight(float Value)
{
    if (Value != 0.0f && !bIsDead)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void AAegisCharacter::FireWeapon()
{
    if (CurrentWeapon && !bIsDead)
    {
        // If we're a client, ask the server to fire
        if (!HasAuthority())
        {
            ServerFireWeapon();
        }
        else
        {
            // We're the server, fire directly
            CurrentWeapon->Fire();
        }
    }
}

void AAegisCharacter::ServerFireWeapon_Implementation()
{
    if (CurrentWeapon && !bIsDead)
    {
        CurrentWeapon->Fire();
    }
}

bool AAegisCharacter::ServerFireWeapon_Validate()
{
    return true; // Basic anti-cheat check
}

float AAegisCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead)
    {
        return 0.0f;
    }

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth -= ActualDamage;
    CurrentHealth = FMath::Max(CurrentHealth, 0.0f);

    UE_LOG(LogTemp, Warning, TEXT("Player took %.2f damage. Health: %.2f / %.2f"),
        ActualDamage, CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return ActualDamage;
}

void AAegisCharacter::Die()
{
    if (bIsDead || !HasAuthority())
    {
        return; // Only server handles death
    }

    bIsDead = true;

    UE_LOG(LogTemp, Warning, TEXT("Player died!"));

    DisableInput(Cast<APlayerController>(GetController()));

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (CurrentWeapon)
    {
        CurrentWeapon->SetActorHiddenInGame(true);
    }

    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

    RemoveSpeedBuff();

    GetWorldTimerManager().SetTimer(
        RespawnTimerHandle,
        this,
        &AAegisCharacter::Respawn,
        RespawnDelay,
        false
    );

    // Tell all clients to play death animation/effects
    MulticastPlayDeathEffects();
}

void AAegisCharacter::MulticastPlayDeathEffects_Implementation()
{
    // This runs on ALL clients
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
    }

    // Play death sound/particles here later
}

void AAegisCharacter::MulticastRespawnEffects_Implementation()
{
    // This runs on ALL clients to fix the mesh
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(false);
        GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        GetMesh()->SetVisibility(true, true);

        // Reattach to capsule in case it got detached
        GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
        GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
        GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    }
}

void AAegisCharacter::Respawn()
{
    if (!HasAuthority())
    {
        return; // Only server can respawn
    }

    UE_LOG(LogTemp, Warning, TEXT("Player respawning..."));

    APlayerStart* PlayerStart = nullptr;
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        PlayerStart = *It;
        break;
    }

    if (PlayerStart)
    {
        SetActorLocation(PlayerStart->GetActorLocation());
        SetActorRotation(PlayerStart->GetActorRotation());
    }

    CurrentHealth = MaxHealth;
    bIsDead = false;

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Fix the mesh properly
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(false);
        GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
        GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
        GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
        GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

        // CRITICAL: Re-enable collision and visibility
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        GetMesh()->SetVisibility(true, true); // Make visible to all
    }

    if (CurrentWeapon)
    {
        CurrentWeapon->SetActorHiddenInGame(false);
    }

    EnableInput(Cast<APlayerController>(GetController()));

    // Tell all clients to fix their version of this character
    MulticastRespawnEffects();

    UE_LOG(LogTemp, Warning, TEXT("Player respawned!"));
}

void AAegisCharacter::OnKillEnemy(FVector KillLocation)
{
    if (bIsDead)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("OnKillEnemy called! Applying speed buff..."));

    PlayEnergyStealFeedback(KillLocation);

    ApplySpeedBuff();
}

void AAegisCharacter::ApplySpeedBuff()
{
    if (!GetCharacterMovement() || bIsDead)
    {
        return;
    }

    float BuffedSpeed = BaseMovementSpeed * (1.0f + SpeedBuffPercentage);
    GetCharacterMovement()->MaxWalkSpeed = BuffedSpeed;

    bIsBuffActive = true;

    UE_LOG(LogTemp, Warning, TEXT("Speed buff applied! Speed: %.2f (was %.2f)"),
        BuffedSpeed, BaseMovementSpeed);

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

    if (BuffActiveSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BuffActiveSound, GetActorLocation());
    }

    GetWorldTimerManager().ClearTimer(BuffTimerHandle);

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

    GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
    bIsBuffActive = false;

    UE_LOG(LogTemp, Warning, TEXT("Speed buff removed! Speed back to: %.2f"), BaseMovementSpeed);

    if (ActiveBuffAuraComponent)
    {
        ActiveBuffAuraComponent->DestroyComponent();
        ActiveBuffAuraComponent = nullptr;
    }
}

void AAegisCharacter::PlayEnergyStealFeedback(FVector KillLocation)
{
    if (KillParticleEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            KillParticleEffect,
            KillLocation,
            FRotator::ZeroRotator,
            FVector(1.5f, 1.5f, 1.5f)
        );

        UE_LOG(LogTemp, Warning, TEXT("Spawned kill particle effect at kill location!"));
    }

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

void AAegisCharacter::UseGrenadeAbility()
{
    if (bIsDead || bGrenadeOnCooldown || !GrenadeClass)
    {
        if (bGrenadeOnCooldown)
        {
            UE_LOG(LogTemp, Warning, TEXT("Grenade on cooldown!"));
        }
        return;
    }

    // Clients ask server to throw grenade
    if (!HasAuthority())
    {
        ServerUseGrenade();
    }
    else
    {
        // Server throws grenade directly
        SpawnAndThrowGrenade();
    }
}

void AAegisCharacter::ServerUseGrenade_Implementation()
{
    if (!bIsDead && !bGrenadeOnCooldown && GrenadeClass)
    {
        SpawnAndThrowGrenade();
    }
}

bool AAegisCharacter::ServerUseGrenade_Validate()
{
    return true;
}

void AAegisCharacter::SpawnAndThrowGrenade()
{
    UE_LOG(LogTemp, Warning, TEXT("Throwing grenade!"));

    AController* OwnerController = GetController();
    if (!OwnerController)
    {
        return;
    }

    FVector CameraLocation;
    FRotator CameraRotation;
    OwnerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector SpawnLocation = CameraLocation + (CameraRotation.Vector() * 100.f);

    FRotator ThrowRotation = CameraRotation;
    ThrowRotation.Pitch += 15.0f;

    FVector ThrowDirection = ThrowRotation.Vector();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    AAegisGrenade* Grenade = GetWorld()->SpawnActor<AAegisGrenade>(
        GrenadeClass,
        SpawnLocation,
        ThrowRotation,
        SpawnParams
    );

    if (Grenade)
    {
        Grenade->ThrowGrenade(ThrowDirection, GrenadeThrowSpeed);
        UE_LOG(LogTemp, Warning, TEXT("Grenade spawned successfully!"));

        bGrenadeOnCooldown = true;
        GetWorldTimerManager().SetTimer(
            GrenadeCooldownTimerHandle,
            this,
            &AAegisCharacter::ResetGrenadeCooldown,
            GrenadeCooldown,
            false
        );
    }
}

void AAegisCharacter::ResetGrenadeCooldown()
{
    bGrenadeOnCooldown = false;
    UE_LOG(LogTemp, Warning, TEXT("Grenade ready!"));
}

float AAegisCharacter::GetGrenadeCooldownRemaining() const
{
    if (!bGrenadeOnCooldown)
    {
        return 0.0f;
    }

    return GetWorldTimerManager().GetTimerRemaining(GrenadeCooldownTimerHandle);
}