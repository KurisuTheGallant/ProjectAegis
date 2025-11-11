#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AegisPlayerController.generated.h"

UCLASS()
class PROJECTAEGIS_API AAegisPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    // Constructor
    AAegisPlayerController();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Add kill notification to feed
    UFUNCTION(Client, Reliable)
    void ClientShowKillNotification(const FString& KillerName, const FString& VictimName);

    // Kill feed messages (last 5 kills)
    UPROPERTY(BlueprintReadOnly, Category = "HUD")
    TArray<FString> KillFeedMessages;

    // Kill feed timestamps (to fade them out)
    UPROPERTY(BlueprintReadOnly, Category = "HUD")
    TArray<float> KillFeedTimestamps;

    // How long kill feed messages stay on screen
    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    float KillFeedDuration;

    // Maximum kill feed messages
    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    int32 MaxKillFeedMessages;

    // Update kill feed (remove old messages)
    void UpdateKillFeed(float DeltaTime);
};