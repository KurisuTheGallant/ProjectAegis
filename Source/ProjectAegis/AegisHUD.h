#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AegisHUD.generated.h"

// Struct for damage numbers
USTRUCT()
struct FDamageNumber
{
    GENERATED_BODY()

    FVector WorldLocation;
    float Damage;
    float TimeRemaining;
    bool bIsKill;
    FVector2D ScreenPosition;

    FDamageNumber()
        : WorldLocation(FVector::ZeroVector)
        , Damage(0.0f)
        , TimeRemaining(0.0f)
        , bIsKill(false)
        , ScreenPosition(FVector2D::ZeroVector)
    {
    }
};

// Struct for hit markers
USTRUCT()
struct FHitMarker
{
    GENERATED_BODY()

    float TimeRemaining;
    bool bIsKill;

    FHitMarker()
        : TimeRemaining(0.0f)
        , bIsKill(false)
    {
    }
};

UCLASS()
class PROJECTAEGIS_API AAegisHUD : public AHUD
{
    GENERATED_BODY()

public:
    AAegisHUD();
    virtual void DrawHUD() override;
    virtual void Tick(float DeltaTime) override;

    // Show hit marker
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowHitMarker(bool bWasKill = false);

    // Show damage number
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowDamageNumber(FVector WorldLocation, float Damage, bool bWasKill = false);

protected:
    virtual void BeginPlay() override;

private:
    // === DRAWING FUNCTIONS ===
    void DrawHealthBar();
    void DrawSpeedBuffIndicator();
    void DrawGrenadeCooldown();
    void DrawTeamScores();
    void DrawKillFeed();
    void DrawCrosshair();
    void DrawHitMarkers();
    void DrawDamageNumbers();
    void DrawLowHealthEffect();
    void DrawKillNotification();

    // === HELPER FUNCTIONS ===
    void DrawProgressBar(float X, float Y, float Width, float Height, float Percent, FLinearColor BarColor, FLinearColor BackgroundColor);
    void DrawTextWithOutline(const FString& Text, float X, float Y, FLinearColor TextColor, float Scale = 1.0f);

    // === HIT MARKER SYSTEM ===
    UPROPERTY()
    TArray<FHitMarker> ActiveHitMarkers;

    UPROPERTY(EditDefaultsOnly, Category = "HUD|HitMarkers")
    float HitMarkerDuration;

    UPROPERTY(EditDefaultsOnly, Category = "HUD|HitMarkers")
    float HitMarkerSize;

    // === DAMAGE NUMBERS ===
    UPROPERTY()
    TArray<FDamageNumber> ActiveDamageNumbers;

    UPROPERTY(EditDefaultsOnly, Category = "HUD|DamageNumbers")
    float DamageNumberDuration;

    UPROPERTY(EditDefaultsOnly, Category = "HUD|DamageNumbers")
    float DamageNumberSpeed;

    // === KILL NOTIFICATION ===
    float KillNotificationTimer;

    UPROPERTY(EditDefaultsOnly, Category = "HUD|Notifications")
    float KillNotificationDuration;

    // === FONTS ===
    UPROPERTY()
    UFont* HUDFont;

    UPROPERTY()
    UFont* LargeFont;

    // === COLORS ===
    FLinearColor HealthBarColor;
    FLinearColor HealthBarBackgroundColor;
    FLinearColor BuffActiveColor;
    FLinearColor GrenadeCooldownColor;
    FLinearColor GrenadeReadyColor;
    FLinearColor TeamScoreColor;
    FLinearColor KillFeedColor;
    FLinearColor CrosshairColor;
    FLinearColor HitMarkerColor;
    FLinearColor KillMarkerColor;
    FLinearColor DamageNumberColor;
    FLinearColor KillDamageNumberColor;
};