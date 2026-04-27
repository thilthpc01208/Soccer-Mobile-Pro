// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MatchState.generated.h"

class AOutfieldCharacter;

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Gameplay events types that can occur during a soccer match. These are sent out by the
///              GameMode to notify other classes. These generally 
//----------------------------------------------------------------------------------------------------
UENUM()
enum EMatchEvent
{
    PlayStateChanged,           // The Play State of the Match has changed.
    StartPlay,                  // The Match has started, meaning Players can now act.
    StopPlay,                   // The Match has stopped, meaning Players can no longer act.
    OutfielderPossessionChange, // Possession of the ball has changed.
    TeamPossessionChange,	    // A new Team (or null) has possession of the ball.
    GoalScored,                 // A Goal has been scored.
    MatchEnd,                   // The Match has concluded.
};

USTRUCT()
struct PROJECTSOCCER_API FGoalScoredInfo
{
    GENERATED_BODY()
	// int32 MinuteScored;
    FVector ShotLocation;
    FGameplayTag ScoringTeam;
    TObjectPtr<AOutfieldCharacter> ScoringOutfielder;
};

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      This is the "single source of truth" for all actors that care about the state of the match, and
//      should be queried.
//		
///		@brief : Data describing the current state of a soccer match.
//----------------------------------------------------------------------------------------------------
USTRUCT()
struct PROJECTSOCCER_API FMatchState
{
    GENERATED_BODY()

    // FFieldState FieldState;
    TArray<FGoalScoredInfo> GoalsScored; // The last element is then most-recent goal scored.
    TObjectPtr<AOutfieldCharacter> OutfielderInPossession = nullptr;
    int32 HomeTeamScore = 0;
    int32 AwayTeamScore = 0;

    FGameplayTag GetTeamInPossessionTag() const;
    void Reset();
};