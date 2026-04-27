// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ProjectSoccer/GameMode/MatchEntityInterface.h"
#include "PossessionTimer.generated.h"

class ASoccerBall;

DECLARE_LOG_CATEGORY_EXTERN(LogPossessionTimer, Log, All);

UCLASS()
class PROJECTSOCCER_API APossessionTimer : public AActor, public IMatchEntityInterface
{
    GENERATED_BODY()

    static constexpr float kPossessionTickInterval = 1.f;

    FTimerHandle TimerHandle;
    float PossessionTimeElapsed = 0.f;

public:
    void StartTimer();
    void ResetTimer();
    void StopTimer();
    float GetTimeElapsed() const;

private:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode) override {}
    virtual void OnMatchPhaseBegin(const EMatchPhase phase) override;
    void OnMatchEvent(const EMatchEvent event, const FMatchState& state);

    void OnPossessionChanged(FGameplayTag tag);
    void UpdateTime();
};
