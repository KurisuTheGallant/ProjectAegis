#include "AegisHUD.h"
#include "AegisCharacter.h"
#include "AegisGameMode.h"
#include "AegisPlayerController.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

AAegisHUD::AAegisHUD()
{
    PrimaryActorTick.bCanEverTick = true;

    // Load fonts
    static ConstructorHelpers::FObjectFinder<UFont> FontObj(TEXT("/Engine/EngineFonts/Roboto"));
    if (FontObj.Succeeded())
    {
        HUDFont = FontObj.Object;
        LargeFont = FontObj.Object;
    }

    // Set colors
    HealthBarColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
    HealthBarBackgroundColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.8f);
    BuffActiveColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);
    GrenadeCooldownColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
    GrenadeReadyColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
    TeamScoreColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    KillFeedColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
    CrosshairColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.8f);
    HitMarkerColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
    KillMarkerColor = FLinearColor(1.0f, 0.843f, 0.0f, 1.0f);
    DamageNumberColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    KillDamageNumberColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

    // Hit marker settings
    HitMarkerDuration = 0.2f;
    HitMarkerSize = 20.0f;

    // Damage number settings
    DamageNumberDuration = 1.5f;
    DamageNumberSpeed = 100.0f;

    // Kill notification
    KillNotificationDuration = 2.0f;
    KillNotificationTimer = 0.0f;
}

void AAegisHUD::BeginPlay()
{
    Super::BeginPlay();
}

void AAegisHUD::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update hit markers
    for (int32 i = ActiveHitMarkers.Num() - 1; i >= 0; i--)
    {
        ActiveHitMarkers[i].TimeRemaining -= DeltaTime;
        if (ActiveHitMarkers[i].TimeRemaining <= 0.0f)
        {
            ActiveHitMarkers.RemoveAt(i);
        }
    }

    // Update damage numbers
    for (int32 i = ActiveDamageNumbers.Num() - 1; i >= 0; i--)
    {
        ActiveDamageNumbers[i].TimeRemaining -= DeltaTime;
        if (ActiveDamageNumbers[i].TimeRemaining <= 0.0f)
        {
            ActiveDamageNumbers.RemoveAt(i);
        }
    }

    // Update kill notification
    if (KillNotificationTimer > 0.0f)
    {
        KillNotificationTimer -= DeltaTime;
    }
}

void AAegisHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas)
    {
        return;
    }

    // Draw all HUD elements
    DrawLowHealthEffect();
    DrawHealthBar();
    DrawSpeedBuffIndicator();
    DrawGrenadeCooldown();
    DrawTeamScores();
    DrawKillFeed();
    DrawHitMarkers();
    DrawDamageNumbers();
    DrawKillNotification();
    DrawCrosshair();
}

void AAegisHUD::ShowHitMarker(bool bWasKill)
{
    FHitMarker NewMarker;
    NewMarker.TimeRemaining = HitMarkerDuration;
    NewMarker.bIsKill = bWasKill;
    ActiveHitMarkers.Add(NewMarker);

    if (bWasKill)
    {
        KillNotificationTimer = KillNotificationDuration;
    }
}

void AAegisHUD::ShowDamageNumber(FVector WorldLocation, float Damage, bool bWasKill)
{
    FDamageNumber NewNumber;
    NewNumber.WorldLocation = WorldLocation;
    NewNumber.Damage = Damage;
    NewNumber.TimeRemaining = DamageNumberDuration;
    NewNumber.bIsKill = bWasKill;
    ActiveDamageNumbers.Add(NewNumber);
}

void AAegisHUD::DrawHealthBar()
{
    AAegisCharacter* Character = Cast<AAegisCharacter>(GetOwningPawn());
    if (!Character)
    {
        return;
    }

    float HealthPercent = Character->GetHealthPercent();

    // Position: Bottom center
    float BarWidth = 400.0f;
    float BarHeight = 30.0f;
    float BarX = (Canvas->SizeX - BarWidth) / 2.0f;
    float BarY = Canvas->SizeY - 100.0f;

    // Draw health bar
    DrawProgressBar(BarX, BarY, BarWidth, BarHeight, HealthPercent, HealthBarColor, HealthBarBackgroundColor);

    // Draw health text
    FString HealthText = FString::Printf(TEXT("%.0f / %.0f HP"), Character->GetCurrentHealth(), Character->GetMaxHealth());
    DrawTextWithOutline(HealthText, BarX + BarWidth / 2.0f - 50.0f, BarY + 5.0f, FLinearColor::White, 1.0f);
}

void AAegisHUD::DrawSpeedBuffIndicator()
{
    AAegisCharacter* Character = Cast<AAegisCharacter>(GetOwningPawn());
    if (!Character || !Character->IsBuffActive())
    {
        return;
    }

    // Position: Center of screen, slightly above crosshair
    float TextX = Canvas->SizeX / 2.0f - 150.0f;
    float TextY = Canvas->SizeY / 2.0f - 100.0f;

    // Pulsing effect
    float PulseScale = 1.0f + FMath::Sin(GetWorld()->GetTimeSeconds() * 10.0f) * 0.1f;

    // Draw buff text with glow effect
    FString BuffText = TEXT("⚡ SPEED BOOST ⚡");
    DrawTextWithOutline(BuffText, TextX, TextY, BuffActiveColor, 1.5f * PulseScale);

    // Draw speed value
    FString SpeedText = FString::Printf(TEXT("Speed: %.0f"), Character->GetCurrentSpeed());
    DrawTextWithOutline(SpeedText, TextX + 50.0f, TextY + 30.0f, BuffActiveColor, 1.0f);
}

void AAegisHUD::DrawGrenadeCooldown()
{
    AAegisCharacter* Character = Cast<AAegisCharacter>(GetOwningPawn());
    if (!Character)
    {
        return;
    }

    // Position: Bottom right
    float IconSize = 60.0f;
    float IconX = Canvas->SizeX - 120.0f;
    float IconY = Canvas->SizeY - 120.0f;

    if (Character->IsGrenadeOnCooldown())
    {
        float CooldownRemaining = Character->GetGrenadeCooldownRemaining();

        // Draw cooldown circle background
        DrawRect(GrenadeCooldownColor, IconX, IconY, IconSize, IconSize);

        // Draw cooldown text
        FString CooldownText = FString::Printf(TEXT("%.1f"), CooldownRemaining);
        DrawTextWithOutline(CooldownText, IconX + 10.0f, IconY + 15.0f, FLinearColor::White, 1.5f);
    }
    else
    {
        // Draw ready indicator with pulse
        float PulseScale = 1.0f + FMath::Sin(GetWorld()->GetTimeSeconds() * 5.0f) * 0.1f;
        FLinearColor PulseColor = GrenadeReadyColor;
        PulseColor.A = 0.8f + FMath::Sin(GetWorld()->GetTimeSeconds() * 5.0f) * 0.2f;

        DrawRect(PulseColor, IconX, IconY, IconSize, IconSize);
        DrawTextWithOutline(TEXT("[Q]"), IconX + 10.0f, IconY + 5.0f, FLinearColor::Black, 1.5f);
        DrawTextWithOutline(TEXT("READY"), IconX, IconY + 35.0f, FLinearColor::Black, 0.8f);
    }
}

void AAegisHUD::DrawTeamScores()
{
    AAegisGameMode* GameMode = Cast<AAegisGameMode>(GetWorld()->GetAuthGameMode());
    if (!GameMode)
    {
        return;
    }

    int32 TeamAScore = GameMode->GetTeamScore(EAegisTeam::TeamA);
    int32 TeamBScore = GameMode->GetTeamScore(EAegisTeam::TeamB);

    // Position: Top center
    float TextX = Canvas->SizeX / 2.0f - 150.0f;
    float TextY = 30.0f;

    FString ScoreText = FString::Printf(TEXT("TEAM A: %d  |  TEAM B: %d"), TeamAScore, TeamBScore);
    DrawTextWithOutline(ScoreText, TextX, TextY, TeamScoreColor, 1.5f);

    // Draw round info
    FString RoundText = FString::Printf(TEXT("Round %d - Rounds Won: %d - %d"),
        GameMode->CurrentRound,
        GameMode->GetTeamRoundsWon(EAegisTeam::TeamA),
        GameMode->GetTeamRoundsWon(EAegisTeam::TeamB));
    DrawTextWithOutline(RoundText, TextX - 50.0f, TextY + 35.0f, TeamScoreColor, 1.0f);
}

void AAegisHUD::DrawKillFeed()
{
    AAegisPlayerController* PC = Cast<AAegisPlayerController>(GetOwningPlayerController());
    if (!PC)
    {
        return;
    }

    // Position: Top right
    float StartX = Canvas->SizeX - 400.0f;
    float StartY = 100.0f;
    float LineHeight = 25.0f;

    for (int32 i = 0; i < PC->KillFeedMessages.Num(); i++)
    {
        // Calculate fade based on age
        float Age = PC->KillFeedTimestamps[i];
        float Alpha = FMath::Clamp(1.0f - (Age / PC->KillFeedDuration), 0.0f, 1.0f);

        FLinearColor FadeColor = KillFeedColor;
        FadeColor.A = Alpha;

        // Slide in effect for new messages
        float SlideOffset = 0.0f;
        if (Age < 0.5f)
        {
            SlideOffset = (1.0f - (Age / 0.5f)) * 100.0f;
        }

        DrawTextWithOutline(PC->KillFeedMessages[i], StartX + SlideOffset, StartY + (i * LineHeight), FadeColor, 1.0f);
    }
}

void AAegisHUD::DrawCrosshair()
{
    // Position: Center of screen
    float CenterX = Canvas->SizeX / 2.0f;
    float CenterY = Canvas->SizeY / 2.0f;
    float CrosshairSize = 10.0f;
    float CrosshairThickness = 2.0f;

    FLinearColor CurrentCrosshairColor = CrosshairColor;

    // Change color if hit marker is active
    if (ActiveHitMarkers.Num() > 0)
    {
        if (ActiveHitMarkers[0].bIsKill)
        {
            CurrentCrosshairColor = KillMarkerColor;
            CrosshairSize *= 1.5f;
        }
        else
        {
            CurrentCrosshairColor = HitMarkerColor;
        }
    }

    // Draw crosshair lines
    DrawRect(CurrentCrosshairColor, CenterX - CrosshairSize, CenterY - CrosshairThickness / 2.0f, CrosshairSize * 2.0f, CrosshairThickness);
    DrawRect(CurrentCrosshairColor, CenterX - CrosshairThickness / 2.0f, CenterY - CrosshairSize, CrosshairThickness, CrosshairSize * 2.0f);
    DrawRect(CurrentCrosshairColor, CenterX - 1.0f, CenterY - 1.0f, 2.0f, 2.0f);
}

void AAegisHUD::DrawHitMarkers()
{
    if (ActiveHitMarkers.Num() == 0)
    {
        return;
    }

    float CenterX = Canvas->SizeX / 2.0f;
    float CenterY = Canvas->SizeY / 2.0f;

    for (const FHitMarker& Marker : ActiveHitMarkers)
    {
        float Alpha = Marker.TimeRemaining / HitMarkerDuration;
        FLinearColor MarkerColor = Marker.bIsKill ? KillMarkerColor : HitMarkerColor;
        MarkerColor.A = Alpha;

        float Size = HitMarkerSize * (Marker.bIsKill ? 1.5f : 1.0f);
        float Thickness = 3.0f;
        float Gap = 5.0f;

        // Draw X-shaped hit marker
        // Top-left to center
        DrawLine(CenterX - Size - Gap, CenterY - Size - Gap, CenterX - Gap, CenterY - Gap, MarkerColor, Thickness);
        // Top-right to center
        DrawLine(CenterX + Gap, CenterY - Gap, CenterX + Size + Gap, CenterY - Size - Gap, MarkerColor, Thickness);
        // Bottom-left to center
        DrawLine(CenterX - Size - Gap, CenterY + Size + Gap, CenterX - Gap, CenterY + Gap, MarkerColor, Thickness);
        // Bottom-right to center
        DrawLine(CenterX + Gap, CenterY + Gap, CenterX + Size + Gap, CenterY + Size + Gap, MarkerColor, Thickness);
    }
}

void AAegisHUD::DrawDamageNumbers()
{
    if (!Canvas)
    {
        return;
    }

    APlayerController* PC = GetOwningPlayerController();
    if (!PC)
    {
        return;
    }

    for (FDamageNumber& DamageNum : ActiveDamageNumbers)
    {
        // Move damage number up over time
        float Progress = 1.0f - (DamageNum.TimeRemaining / DamageNumberDuration);
        FVector MovedLocation = DamageNum.WorldLocation + FVector(0, 0, DamageNumberSpeed * Progress);

        // Convert world to screen
        FVector2D ScreenPos;
        if (PC->ProjectWorldLocationToScreen(MovedLocation, ScreenPos))
        {
            // Fade out over time
            float Alpha = DamageNum.TimeRemaining / DamageNumberDuration;
            FLinearColor NumberColor = DamageNum.bIsKill ? KillDamageNumberColor : DamageNumberColor;
            NumberColor.A = Alpha;

            // Scale based on if it's a kill
            float Scale = DamageNum.bIsKill ? 2.0f : 1.5f;

            FString DamageText = FString::Printf(TEXT("%.0f"), DamageNum.Damage);
            if (DamageNum.bIsKill)
            {
                DamageText = FString::Printf(TEXT("💀 %.0f 💀"), DamageNum.Damage);
            }

            DrawTextWithOutline(DamageText, ScreenPos.X - 20.0f, ScreenPos.Y, NumberColor, Scale);
        }
    }
}

void AAegisHUD::DrawLowHealthEffect()
{
    AAegisCharacter* Character = Cast<AAegisCharacter>(GetOwningPawn());
    if (!Character)
    {
        return;
    }

    float HealthPercent = Character->GetHealthPercent();

    // Only show if health is below 30%
    if (HealthPercent > 0.3f)
    {
        return;
    }

    // Pulsing red vignette
    float PulseIntensity = (0.3f - HealthPercent) / 0.3f; // 0 at 30% health, 1 at 0% health
    float Pulse = FMath::Sin(GetWorld()->GetTimeSeconds() * 5.0f) * 0.3f + 0.7f;
    float Alpha = PulseIntensity * Pulse * 0.5f;

    FLinearColor VignetteColor = FLinearColor(1.0f, 0.0f, 0.0f, Alpha);

    // Draw vignette edges
    float VignetteSize = 200.0f;

    // Top
    DrawRect(VignetteColor, 0, 0, Canvas->SizeX, VignetteSize);
    // Bottom
    DrawRect(VignetteColor, 0, Canvas->SizeY - VignetteSize, Canvas->SizeX, VignetteSize);
    // Left
    DrawRect(VignetteColor, 0, 0, VignetteSize, Canvas->SizeY);
    // Right
    DrawRect(VignetteColor, Canvas->SizeX - VignetteSize, 0, VignetteSize, Canvas->SizeY);
}

void AAegisHUD::DrawKillNotification()
{
    if (KillNotificationTimer <= 0.0f)
    {
        return;
    }

    float CenterX = Canvas->SizeX / 2.0f;
    float CenterY = Canvas->SizeY / 2.0f + 100.0f;

    // Fade and scale based on time
    float Progress = KillNotificationTimer / KillNotificationDuration;
    float Alpha = FMath::Min(Progress * 2.0f, 1.0f); // Fade in quickly
    float Scale = 2.0f + (1.0f - Progress) * 0.5f; // Start big, get slightly smaller

    FLinearColor KillColor = FLinearColor(1.0f, 0.843f, 0.0f, Alpha);

    DrawTextWithOutline(TEXT("💀 ELIMINATION 💀"), CenterX - 150.0f, CenterY, KillColor, Scale);
}

void AAegisHUD::DrawProgressBar(float X, float Y, float Width, float Height, float Percent, FLinearColor BarColor, FLinearColor BackgroundColor)
{
    // Draw background
    DrawRect(BackgroundColor, X, Y, Width, Height);

    // Draw filled portion
    float FilledWidth = Width * FMath::Clamp(Percent, 0.0f, 1.0f);
    DrawRect(BarColor, X, Y, FilledWidth, Height);

    // Draw border
    float BorderThickness = 2.0f;
    DrawRect(FLinearColor::Black, X, Y, Width, BorderThickness);
    DrawRect(FLinearColor::Black, X, Y + Height - BorderThickness, Width, BorderThickness);
    DrawRect(FLinearColor::Black, X, Y, BorderThickness, Height);
    DrawRect(FLinearColor::Black, X + Width - BorderThickness, Y, BorderThickness, Height);
}

void AAegisHUD::DrawTextWithOutline(const FString& Text, float X, float Y, FLinearColor TextColor, float Scale)
{
    if (!HUDFont)
    {
        return;
    }

    // Draw outline
    FLinearColor OutlineColor = FLinearColor::Black;
    OutlineColor.A = TextColor.A;
    float OutlineOffset = 1.0f;

    DrawText(Text, OutlineColor, X - OutlineOffset, Y - OutlineOffset, HUDFont, Scale, false);
    DrawText(Text, OutlineColor, X + OutlineOffset, Y - OutlineOffset, HUDFont, Scale, false);
    DrawText(Text, OutlineColor, X - OutlineOffset, Y + OutlineOffset, HUDFont, Scale, false);
    DrawText(Text, OutlineColor, X + OutlineOffset, Y + OutlineOffset, HUDFont, Scale, false);

    // Draw main text
    DrawText(Text, TextColor, X, Y, HUDFont, Scale, false);
}