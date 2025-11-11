#include "AegisPlayerController.h"

AAegisPlayerController::AAegisPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;

    KillFeedDuration = 5.0f;
    MaxKillFeedMessages = 5;
}

void AAegisPlayerController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("AegisPlayerController: Started"));
}

void AAegisPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateKillFeed(DeltaTime);
}

void AAegisPlayerController::ClientShowKillNotification_Implementation(const FString& KillerName, const FString& VictimName)
{
    FString Message = FString::Printf(TEXT("%s killed %s"), *KillerName, *VictimName);

    // Add to kill feed
    KillFeedMessages.Insert(Message, 0); // Add to front
    KillFeedTimestamps.Insert(0.0f, 0); // Reset timestamp

    // Limit to max messages
    if (KillFeedMessages.Num() > MaxKillFeedMessages)
    {
        KillFeedMessages.SetNum(MaxKillFeedMessages);
        KillFeedTimestamps.SetNum(MaxKillFeedMessages);
    }

    UE_LOG(LogTemp, Warning, TEXT("Kill Feed: %s"), *Message);
}

void AAegisPlayerController::UpdateKillFeed(float DeltaTime)
{
    // Increment all timestamps
    for (int32 i = 0; i < KillFeedTimestamps.Num(); i++)
    {
        KillFeedTimestamps[i] += DeltaTime;
    }

    // Remove expired messages
    for (int32 i = KillFeedTimestamps.Num() - 1; i >= 0; i--)
    {
        if (KillFeedTimestamps[i] > KillFeedDuration)
        {
            KillFeedMessages.RemoveAt(i);
            KillFeedTimestamps.RemoveAt(i);
        }
    }
}