#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AegisHUD.generated.h"

UCLASS()
class PROJECTAEGIS_API AAegisHUD : public AHUD
{
    GENERATED_BODY()

public:
    // Constructor
    AAegisHUD();

    // Main HUD draw function
    virtual void DrawHUD() override;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

private:
    // === DRAWING FUNCTIONS ===

    // Draw health bar
    void DrawHealthBar();

    // Draw speed buff indicator
    void DrawSpeedBuffIndicator();

    // Draw grenade cooldown
    void DrawGrenadeCooldown();

    // Draw team scores
    void DrawTeamScores();

    // Draw kill feed
    void DrawKillFeed();

    // Draw crosshair
    void DrawCrosshair();

    // === HELPER FUNCTIONS ===

    // Draw a progress bar
    void DrawProgressBar(float X, float Y, float Width, float Height, float Percent, FLinearColor BarColor, FLinearColor BackgroundColor);

    // Draw text with outline
    void DrawTextWithOutline(const FString& Text, float X, float Y, FLinearColor TextColor, float Scale = 1.0f);

    // === FONTS ===

    UPROPERTY()
    UFont* HUDFont;

    // === COLORS ===

    FLinearColor HealthBarColor;
    FLinearColor HealthBarBackgroundColor;
    FLinearColor BuffActiveColor;
    FLinearColor GrenadeCooldownColor;
    FLinearColor GrenadeReadyColor;
    FLinearColor TeamScoreColor;
    FLinearColor KillFeedColor;
    FLinearColor CrosshairColor;
};