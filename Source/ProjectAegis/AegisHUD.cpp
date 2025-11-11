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
    // Load default font
    static ConstructorHelpers::FObjectFinder<UFont> FontObj(TEXT("/Engine/EngineFonts/Roboto"));
    if (FontObj.Succeeded())
    {
        HUDFont = FontObj.Object;
    }

    // Set colors
    HealthBarColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
    HealthBarBackgroundColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.8f); // Dark gray
    BuffActiveColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f); // Cyan
    GrenadeCooldownColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // Gray
    GrenadeReadyColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
    TeamScoreColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White
    KillFeedColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Gold
    CrosshairColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.8f); // White
}

void AAegisHUD::BeginPlay()
{
    Super::BeginPlay();
}

void AAegisHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas)
    {
        return;
    }

    // Draw all HUD elements
    DrawHealthBar();
    DrawSpeedBuffIndicator();
    DrawGrenadeCooldown();
    DrawTeamScores();
    DrawKillFeed();
    DrawCrosshair();
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

    // Draw health text - 🟢 FIXED: Using GetMaxHealth() instead of MaxHealth
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

    // Draw buff text with glow effect
    FString BuffText = TEXT("SPEED BOOST ACTIVE!");
    DrawTextWithOutline(BuffText, TextX, TextY, BuffActiveColor, 1.5f);

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
        // Draw ready indicator
        DrawRect(GrenadeReadyColor, IconX, IconY, IconSize, IconSize);
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

        DrawTextWithOutline(PC->KillFeedMessages[i], StartX, StartY + (i * LineHeight), FadeColor, 1.0f);
    }
}

void AAegisHUD::DrawCrosshair()
{
    // Position: Center of screen
    float CenterX = Canvas->SizeX / 2.0f;
    float CenterY = Canvas->SizeY / 2.0f;
    float CrosshairSize = 10.0f;
    float CrosshairThickness = 2.0f;

    // Draw crosshair lines
    // Horizontal line
    DrawRect(CrosshairColor, CenterX - CrosshairSize, CenterY - CrosshairThickness / 2.0f, CrosshairSize * 2.0f, CrosshairThickness);

    // Vertical line
    DrawRect(CrosshairColor, CenterX - CrosshairThickness / 2.0f, CenterY - CrosshairSize, CrosshairThickness, CrosshairSize * 2.0f);

    // Draw center dot
    DrawRect(CrosshairColor, CenterX - 1.0f, CenterY - 1.0f, 2.0f, 2.0f);
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
    DrawRect(FLinearColor::Black, X, Y, Width, BorderThickness); // Top
    DrawRect(FLinearColor::Black, X, Y + Height - BorderThickness, Width, BorderThickness); // Bottom
    DrawRect(FLinearColor::Black, X, Y, BorderThickness, Height); // Left
    DrawRect(FLinearColor::Black, X + Width - BorderThickness, Y, BorderThickness, Height); // Right
}

void AAegisHUD::DrawTextWithOutline(const FString& Text, float X, float Y, FLinearColor TextColor, float Scale)
{
    if (!HUDFont)
    {
        return;
    }

    // Draw outline (black text slightly offset in all directions)
    FLinearColor OutlineColor = FLinearColor::Black;
    float OutlineOffset = 1.0f;

    DrawText(Text, OutlineColor, X - OutlineOffset, Y - OutlineOffset, HUDFont, Scale, false);
    DrawText(Text, OutlineColor, X + OutlineOffset, Y - OutlineOffset, HUDFont, Scale, false);
    DrawText(Text, OutlineColor, X - OutlineOffset, Y + OutlineOffset, HUDFont, Scale, false);
    DrawText(Text, OutlineColor, X + OutlineOffset, Y + OutlineOffset, HUDFont, Scale, false);

    // Draw main text
    DrawText(Text, TextColor, X, Y, HUDFont, Scale, false);
}