// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FieldState.h"
#include "InfluenceMap.h"
#include "GameFramework/Actor.h"
#include "ProjectSoccer/GameMode/MatchEntityInterface.h"
#include "FieldManager.generated.h"

class AOutfieldCharacter;
class AGoalTrigger;
class ATeamManager;
class UFieldManagerDebugComponent;

// TODO: Should this be moved to a more central location? Like the GameMode?
//       - The idea would be that there are multiple debug views across the FieldManager, GameMode, etc.
//UENUM(BlueprintType)
//enum EFieldDebugView : uint8
//{
//    None,			     // No Debug View
//    FieldZones,          // TODO: This should show the field breakdown in zones.
//    OutfielderInfluence, // Shows the current Influence Map values.
//    //StatInfluence,
//};

DECLARE_EVENT_OneParam(AFieldManager, FFieldStateUpdated, const UFieldState& /*FieldState*/);
DECLARE_EVENT_TwoParams(AFieldManager, FFieldDimensionsUpdated, const FVector& /*FieldSize*/, const TArray<FBox>& /*influenceGrid*/);

//----------------------------------------------------------------------------------------------------
///		@brief : Information about the Outfielder's current dynamic State.
//----------------------------------------------------------------------------------------------------
struct FOutfielderSnapshot
{
    FVector WorldLocation;
    FVector PassTargetLocation;
    FOutfielderId Id;
    float InfluenceValue;
    float InfluenceRadius;
};

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : This struct contains a "snapshot" of the current player positions, ball position, etc.
///         This is used to build the FieldState object.
//----------------------------------------------------------------------------------------------------
struct FFieldSnapshot
{
    // TODO: Goalies could probably be included in here as well. 
    TArray<FOutfielderSnapshot> Outfielders;

    // TODO: Ball Power, Owner, etc.
    FVector BallPosition;
};

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      - This should run as fast as it can. It should give the Calculated Field state to the TeamManager
//        as soon as it is ready, rather than the TeamManager polling for it. Send it by copy.
//		
///		@brief : Builds the FieldState object that represents the positional data of the field so that
///              the AI & TeamManager can make informed decisions.
//----------------------------------------------------------------------------------------------------
UCLASS()
class PROJECTSOCCER_API AFieldManager : public AActor, public IMatchEntityInterface
{
	GENERATED_BODY()

    UFieldState* BuildingState; // The FieldState that is being built on a Thread.
    UFieldState* ReadyState;    // The FieldState that is ready to be used by the AI & TeamManager.
    
    UPROPERTY(EditAnywhere, Category = "FieldManager", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AGoalTrigger> HomeGoal;

    UPROPERTY(EditAnywhere, Category = "FieldManager", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AGoalTrigger> AwayGoal;

    UPROPERTY()
    TObjectPtr<ATeamManager> HomeTeamManager = nullptr;

    UPROPERTY()
    TObjectPtr<ATeamManager> AwayTeamManager = nullptr;

    UPROPERTY(Category = "FieldManager|Field", EditInstanceOnly)
    FVector2D FieldSize{};

    /*The density of tiles within the InfluenceMap's volume, in the X and Y direction*/
    UPROPERTY(Category = "FieldManager|Field", EditInstanceOnly)
    FIntPoint InfluenceMapResolution{};

    // TODO: Zone Sizes...
    TArray<TObjectPtr<AOutfieldCharacter>> Outfielders;
	UE::Tasks::FTask FieldStateTask;
    TObjectPtr<AProjectSoccerGameMode> GameMode;
    FGameplayTag HomeTag;
    FGameplayTag AwayTag;

    FFieldStateUpdated OnFieldStateUpdatedEvent;
    FFieldDimensionsUpdated OnFieldDimensionsUpdatedEvent;

public:	
	// Sets default values for this actor's properties
	AFieldManager();

    const UFieldState* GetFieldState() const { return ReadyState; }
    FVector GetFieldSize() const { return FVector(FieldSize.X, FieldSize.Y, 1.f); }

    FFieldDimensionsUpdated& OnFieldDimensionsUpdated() { return OnFieldDimensionsUpdatedEvent; }
    FFieldStateUpdated& OnFieldStateUpdated() { return OnFieldStateUpdatedEvent; }

private:
	virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode) override;
    virtual void OnMatchPhaseBegin(const EMatchPhase phase) override;
    void OnMatchPhaseRequested(const TObjectPtr<AProjectSoccerGameMode>& pGameMode, const EMatchPhase phase);

    void BeginBuildFieldState();
    void CheckFieldStateReady();

    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    static void BuildFieldState(UFieldState* pBuildingState, const FFieldSnapshot& snapshot);
    static void SetInTeamHalf(const UFieldState* pBuildingState, const FFieldSnapshot& snapshot, FOutfielderStateInfo& stateInfo);
};

//class FAsyncFieldStateBuilder : public FNonAbandonableTask
//{
//    //FGameState* pGameState = nullptr;
//
//public:
//    FAsyncFieldStateBuilder(/*FGameState* pGameState*/) {} //: pGameState(pGameState) {}
//
//    // Required Function for the AsyncTask.
//    FORCEINLINE TStatId GetStatId() const
//    {
//        RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncFieldStateBuilder, STATGROUP_ThreadPoolAsyncTasks);
//    }
//
//    void DoWork();
//};