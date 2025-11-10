#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AegisGameMode.generated.h"

// Forward declaration
class AAegisCharacter;

// Enum for team identification
UENUM(BlueprintType)
enum class EAegisTeam : uint8
{
    None = 0,
    TeamA = 1,
    TeamB = 2
};

UCLASS()
class PROJECTAEGIS_API AAegisGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    // Constructor
    AAegisGameMode();

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Called when player joins
    virtual void PostLogin(APlayerController* NewPlayer) override;

    // === PUBLIC SCORING API ===

    // Called when a player gets a kill
    UFUNCTION(BlueprintCallable, Category = "Scoring")
    void OnPlayerKill(AController* Killer, AController* Victim);

    // 🟢 NEW FUNCTION - Called when a character kills another character
    UFUNCTION(BlueprintCallable, Category = "Scoring")
    void OnPlayerKilled(AAegisCharacter* Killer, AAegisCharacter* Victim);

    // Get team score
    UFUNCTION(BlueprintPure, Category = "Scoring")
    int32 GetTeamScore(EAegisTeam Team) const;

    // Get team rounds won
    UFUNCTION(BlueprintPure, Category = "Scoring")
    int32 GetTeamRoundsWon(EAegisTeam Team) const;

    // Get player's team
    UFUNCTION(BlueprintCallable, Category = "Teams")
    EAegisTeam GetPlayerTeam(AController* PlayerController) const;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    // === TEAM SYSTEM ===

    // Team A score
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Teams")
    int32 TeamAScore;

    // Team B score
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Teams")
    int32 TeamBScore;

    // Kills needed to win a round
    UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
    int32 KillsToWinRound;

    // Rounds needed to win match
    UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
    int32 RoundsToWinMatch;

    // Current round number
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Game Rules")
    int32 CurrentRound;

    // Team A rounds won
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Game Rules")
    int32 TeamARoundsWon;

    // Team B rounds won
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Game Rules")
    int32 TeamBRoundsWon;

    // Is match over?
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Game Rules")
    bool bMatchOver;

    // Is round active?
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Game Rules")
    bool bRoundActive;

    // === PLAYER MANAGEMENT ===

    // List of Team A players
    UPROPERTY()
    TArray<AController*> TeamAPlayers;

    // List of Team B players
    UPROPERTY()
    TArray<AController*> TeamBPlayers;

    // Assign player to a team
    void AssignPlayerToTeam(AController* PlayerController);

    // === SPAWNING ===

    // Override spawn location selection
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    // === INTERNAL SCORING LOGIC ===

    // Check if round should end
    void CheckRoundEnd();

    // End the current round
    UFUNCTION()
    void EndRound(EAegisTeam WinningTeam);

    // Start a new round
    UFUNCTION()
    void StartNewRound();

    // End the match
    UFUNCTION()
    void EndMatch(EAegisTeam WinningTeam);
};