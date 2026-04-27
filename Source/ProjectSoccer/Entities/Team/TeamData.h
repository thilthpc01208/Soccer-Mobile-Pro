// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Formation.h"
#include "GameplayTagContainer.h"
#include "TeamData.generated.h"

class AOutfieldPlayerController;
class AGoalTrigger;
class AOutfieldCharacter;

// TODO: Remove:
UCLASS()
class PROJECTSOCCER_API UTeamData : public UDataAsset
{
    GENERATED_BODY()

public:
	UPROPERTY(Category = Team, EditAnywhere)
    TSubclassOf<AOutfieldCharacter> PlayerClass;

    UPROPERTY(Category = Team, EditAnywhere)
    UMaterialInstance* TeamMaterial = nullptr;

    UPROPERTY(Category = Team, EditAnywhere)
    TObjectPtr<UFormation> AttackFormation = nullptr;

    UPROPERTY(Category = Team, EditAnywhere)
    TObjectPtr<UFormation> DefenseFormation = nullptr;
};

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Struct containing the data needed to create a Team in the Soccer Game Mode.
///            This is created in Menus when choosing a Team and its players, and sent to the
///            Game Mode to create the Team.
//----------------------------------------------------------------------------------------------------
USTRUCT()
struct PROJECTSOCCER_API FSoccerGameTeamConfig
{
    GENERATED_BODY()

    // Players assigned to this Team.
    UPROPERTY(Category = Team, VisibleAnywhere)
    TArray<TObjectPtr<AOutfieldPlayerController>> Players;

    // Team Material (Color).
    UPROPERTY(Category = Team, VisibleAnywhere)
    UMaterialInstance* TeamMaterial = nullptr;

    // The Team's Tag, Home or Away.
    UPROPERTY(Category = Team, VisibleAnywhere)
    FGameplayTag TeamTag;

    // Number of Outfielders in the Team.
    UPROPERTY(Category = Team, EditAnywhere)
    int32 TeamSize = 0;
};

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Runtime struct containing info about for a Team, including cosmetics, Home vs Away,
///             Players assigned to the team, etc. Used by the TeamManager during the Soccer Game Mode.
//----------------------------------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FRuntimeTeamData 
{
	GENERATED_BODY()

    UPROPERTY(Category = Team, VisibleAnywhere)
    FGameFormationSet Formations;

    UPROPERTY(Category = Team, VisibleAnywhere)
    TArray<TObjectPtr<AOutfieldPlayerController>> Players;

    // TODO: In the future, this would be an array of chosen player classes.
    UPROPERTY(Category = Team, VisibleAnywhere)
    TSubclassOf<AOutfieldCharacter> DefaultPlayerClass;

    UPROPERTY(Category = Team, VisibleAnywhere)
    TObjectPtr<AGoalTrigger> TeamGoal;

    UPROPERTY(Category = Team, VisibleAnywhere)
    TObjectPtr<AGoalTrigger> OpponentGoal;

    UPROPERTY(Category = Team, VisibleAnywhere)
    UMaterialInstance* TeamMaterial = nullptr;

    UPROPERTY(Category = Team, VisibleAnywhere)
    FGameplayTag TeamTag = FGameplayTag::EmptyTag;

    UPROPERTY(Category = Team, VisibleAnywhere)
    int32 TeamSize = 0;

    UPROPERTY(Category = Team, VisibleAnywhere)
    float InfluenceValue = 0.f;
};