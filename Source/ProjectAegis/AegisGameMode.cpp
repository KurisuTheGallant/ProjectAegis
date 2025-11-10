#include "AegisGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "AegisCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

AAegisGameMode::AAegisGameMode()
{
    // Find and set the default pawn class
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
        UE_LOG(LogTemp, Warning, TEXT("AegisGameMode: Default pawn class set successfully!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AegisGameMode: Failed to find default pawn class! Check the path."));
    }

    // Game rules
    KillsToWinRound = 10;
    RoundsToWinMatch = 2; // Best of 3

    // Initialize scores
    TeamAScore = 0;
    TeamBScore = 0;
    CurrentRound = 1;
    TeamARoundsWon = 0;
    TeamBRoundsWon = 0;
    bMatchOver = false;
    bRoundActive = true;

    // Enable replication
    bReplicates = true;
}

void AAegisGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAegisGameMode, TeamAScore);
    DOREPLIFETIME(AAegisGameMode, TeamBScore);
    DOREPLIFETIME(AAegisGameMode, CurrentRound);
    DOREPLIFETIME(AAegisGameMode, TeamARoundsWon);
    DOREPLIFETIME(AAegisGameMode, TeamBRoundsWon);
    DOREPLIFETIME(AAegisGameMode, bMatchOver);
    DOREPLIFETIME(AAegisGameMode, bRoundActive);
}

void AAegisGameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("AegisGameMode: Match started! Round %d"), CurrentRound);
}

void AAegisGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (NewPlayer)
    {
        AssignPlayerToTeam(NewPlayer);
        UE_LOG(LogTemp, Warning, TEXT("Player joined and assigned to team"));
    }
}

void AAegisGameMode::AssignPlayerToTeam(AController* PlayerController)
{
    if (!PlayerController)
    {
        return;
    }

    // Assign to team with fewer players
    if (TeamAPlayers.Num() <= TeamBPlayers.Num())
    {
        TeamAPlayers.Add(PlayerController);
        UE_LOG(LogTemp, Warning, TEXT("Player assigned to Team A (Total: %d)"), TeamAPlayers.Num());
    }
    else
    {
        TeamBPlayers.Add(PlayerController);
        UE_LOG(LogTemp, Warning, TEXT("Player assigned to Team B (Total: %d)"), TeamBPlayers.Num());
    }
}

EAegisTeam AAegisGameMode::GetPlayerTeam(AController* PlayerController) const
{
    if (TeamAPlayers.Contains(PlayerController))
    {
        return EAegisTeam::TeamA;
    }
    else if (TeamBPlayers.Contains(PlayerController))
    {
        return EAegisTeam::TeamB;
    }

    return EAegisTeam::None;
}

AActor* AAegisGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    // Find all player starts in the level
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

    if (PlayerStarts.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No PlayerStart actors found in level!"));
        return nullptr;
    }

    // Get player's team
    EAegisTeam PlayerTeam = GetPlayerTeam(Player);

    // For now, just alternate between first two player starts
    // Team A uses first spawn, Team B uses second spawn
    if (PlayerTeam == EAegisTeam::TeamA && PlayerStarts.Num() > 0)
    {
        return PlayerStarts[0];
    }
    else if (PlayerTeam == EAegisTeam::TeamB && PlayerStarts.Num() > 1)
    {
        return PlayerStarts[1];
    }

    // Fallback to random spawn
    return PlayerStarts[0];
}

// 🟢 NEW FUNCTION - Handle character-to-character kills
void AAegisGameMode::OnPlayerKilled(AAegisCharacter* Killer, AAegisCharacter* Victim)
{
    if (!Killer || !Victim)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerKilled: Invalid Killer or Victim!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("GameMode: %s killed %s"),
        *Killer->GetName(),
        *Victim->GetName());

    // Get the victim's location for visual effects
    FVector KillLocation = Victim->GetActorLocation();

    // Call the killer's OnKillEnemy function to apply speed buff
    Killer->OnKillEnemy(KillLocation);

    UE_LOG(LogTemp, Warning, TEXT("Speed buff applied to killer!"));

    // Also update team scores if you want
    AController* KillerController = Killer->GetController();
    AController* VictimController = Victim->GetController();

    if (KillerController && VictimController)
    {
        OnPlayerKill(KillerController, VictimController);
    }
}

void AAegisGameMode::OnPlayerKill(AController* Killer, AController* Victim)
{
    if (!bRoundActive || bMatchOver)
    {
        return;
    }

    // Get killer's team
    EAegisTeam KillerTeam = GetPlayerTeam(Killer);

    // If victim is nullptr (like a dummy), still award the kill
    if (Victim != nullptr)
    {
        EAegisTeam VictimTeam = GetPlayerTeam(Victim);

        // Don't count team kills
        if (KillerTeam == VictimTeam)
        {
            UE_LOG(LogTemp, Warning, TEXT("Team kill - no points awarded"));
            return;
        }
    }

    // Don't count kills if killer has no team
    if (KillerTeam == EAegisTeam::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("Killer has no team - no points awarded"));
        return;
    }

    // Award point to killer's team
    if (KillerTeam == EAegisTeam::TeamA)
    {
        TeamAScore++;
        UE_LOG(LogTemp, Warning, TEXT("Team A scored! Score: %d - %d"), TeamAScore, TeamBScore);
    }
    else if (KillerTeam == EAegisTeam::TeamB)
    {
        TeamBScore++;
        UE_LOG(LogTemp, Warning, TEXT("Team B scored! Score: %d - %d"), TeamAScore, TeamBScore);
    }

    // Check if round should end
    CheckRoundEnd();
}

void AAegisGameMode::CheckRoundEnd()
{
    // Check if either team reached the kill goal
    if (TeamAScore >= KillsToWinRound)
    {
        EndRound(EAegisTeam::TeamA);
    }
    else if (TeamBScore >= KillsToWinRound)
    {
        EndRound(EAegisTeam::TeamB);
    }
}

void AAegisGameMode::EndRound(EAegisTeam WinningTeam)
{
    if (!bRoundActive)
    {
        return;
    }

    bRoundActive = false;

    UE_LOG(LogTemp, Warning, TEXT("Round %d ended! Winner: Team %s"),
        CurrentRound,
        WinningTeam == EAegisTeam::TeamA ? TEXT("A") : TEXT("B"));

    // Award round win
    if (WinningTeam == EAegisTeam::TeamA)
    {
        TeamARoundsWon++;
    }
    else if (WinningTeam == EAegisTeam::TeamB)
    {
        TeamBRoundsWon++;
    }

    // Check if match is over
    if (TeamARoundsWon >= RoundsToWinMatch)
    {
        EndMatch(EAegisTeam::TeamA);
        return;
    }
    else if (TeamBRoundsWon >= RoundsToWinMatch)
    {
        EndMatch(EAegisTeam::TeamB);
        return;
    }

    // Start new round after 5 seconds
    FTimerHandle RoundRestartTimer;
    GetWorldTimerManager().SetTimer(
        RoundRestartTimer,
        this,
        &AAegisGameMode::StartNewRound,
        5.0f,
        false
    );
}

void AAegisGameMode::StartNewRound()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting new round..."));

    // Reset scores
    TeamAScore = 0;
    TeamBScore = 0;
    CurrentRound++;
    bRoundActive = true;

    // Respawn all players
    for (AController* Controller : TeamAPlayers)
    {
        if (Controller && Controller->GetPawn())
        {
            AAegisCharacter* Character = Cast<AAegisCharacter>(Controller->GetPawn());
            if (Character && Character->IsDead())
            {
                // Force respawn
                Character->Destroy();
                RestartPlayer(Controller);
            }
        }
    }

    for (AController* Controller : TeamBPlayers)
    {
        if (Controller && Controller->GetPawn())
        {
            AAegisCharacter* Character = Cast<AAegisCharacter>(Controller->GetPawn());
            if (Character && Character->IsDead())
            {
                // Force respawn
                Character->Destroy();
                RestartPlayer(Controller);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Round %d started!"), CurrentRound);
}

void AAegisGameMode::EndMatch(EAegisTeam WinningTeam)
{
    bMatchOver = true;
    bRoundActive = false;

    UE_LOG(LogTemp, Warning, TEXT("MATCH OVER! Winner: Team %s (Rounds: %d - %d)"),
        WinningTeam == EAegisTeam::TeamA ? TEXT("A") : TEXT("B"),
        TeamARoundsWon,
        TeamBRoundsWon);

    // TODO: Show match end screen, return to lobby, etc.
}

int32 AAegisGameMode::GetTeamScore(EAegisTeam Team) const
{
    if (Team == EAegisTeam::TeamA)
    {
        return TeamAScore;
    }
    else if (Team == EAegisTeam::TeamB)
    {
        return TeamBScore;
    }

    return 0;
}

int32 AAegisGameMode::GetTeamRoundsWon(EAegisTeam Team) const
{
    if (Team == EAegisTeam::TeamA)
    {
        return TeamARoundsWon;
    }
    else if (Team == EAegisTeam::TeamB)
    {
        return TeamBRoundsWon;
    }

    return 0;
}