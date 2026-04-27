// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MatchEntityInterface.h"
#include "ProjectSoccer/Entities/Team/Formation.h"
#include "ProjectSoccer/Entities/Team/TeamData.h"
#include "GameFramework/GameModeBase.h"
#include "ProjectSoccer/Tools/DebugViews/DebugViewManager.h"
#include "ProjectSoccerGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSoccerGameMode, Log, All);

DECLARE_EVENT_OneParam(AProjectSoccerGameMode, FOnMatchInit, const TObjectPtr<AProjectSoccerGameMode>& /*pGameMode*/);
DECLARE_EVENT_TwoParams(AProjectSoccerGameMode, FOnMatchPhaseRequested, const TObjectPtr<AProjectSoccerGameMode>& /*pGameMode*/, EMatchPhase /*requestedPhase*/);
DECLARE_EVENT_OneParam(AProjectSoccerGameMode, FOnMatchPhaseBegin, EMatchPhase /*phase*/);
DECLARE_EVENT_TwoParams(AProjectSoccerGameMode, FOnMatchEvent, EMatchEvent /*eventType*/, const FMatchState& /*matchState*/);

class APossessionTimer;
class ATeamManager;
class AOutfieldPlayerController;
struct FOutfielderId;
class AGameCamera;
class ASoccerBall;

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Struct containing some default information about a team. Used for testing.
//----------------------------------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FGameModeDefaultConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Home Team")
    UMaterialInstance* HomeMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category = "Away Team")
    UMaterialInstance* AwayMaterial = nullptr;

    UPROPERTY(EditAnywhere)
    TSubclassOf<AOutfieldCharacter> OutfielderClass;

    UPROPERTY(EditAnywhere)
    int32 MaxTeamSize = 2;

    UPROPERTY(EditAnywhere, Category = "Home Team")
    int32 HomeTeamSize = 0;

    UPROPERTY(EditAnywhere, Category = "Away Team")
    int32 AwayTeamSize = 0;
};

UCLASS(minimalapi)
class AProjectSoccerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
    static constexpr unsigned int kMaxOutfielderCount = 2; // TODO: Should be 4.
    static constexpr float kTimeToRestart = 1.f;

    // [TODO]: Make sure that this is a non-shipping only feature.
    // I probably need to remove it from the GameMode, and have it as a spawnable object
    // that can be spawned in the cpp of the GameMode or something.
    UPROPERTY(EditDefaultsOnly, Category = SoccerGame, meta = (RowType="/Script/ProjectSoccer.DebugView"))
    UDataTable* DebugViewsTable;

    UPROPERTY(EditAnywhere, Category = SoccerGame)
    TSubclassOf<ASoccerBall> SoccerBallClass;

    UPROPERTY(EditAnywhere, Category = SoccerGame)
    TSubclassOf<AGameCamera> GameCameraClass;

    UPROPERTY(EditAnywhere, Category = SoccerGame)
    TObjectPtr<UFormationSet> FormationSet = nullptr;

    UPROPERTY(EditAnywhere, Category = "SoccerGame|Home Team", meta = (DisplayAfter = "HomeTeamTag"))
    TObjectPtr<UTeamData> HomeTeamData;
    
    UPROPERTY(EditAnywhere, Category = "SoccerGame|Away Team", meta = (DisplayAfter = "AwayTeamTag"))
    TObjectPtr<UTeamData> AwayTeamData;

    UPROPERTY()
    TObjectPtr<ASoccerBall> SoccerBall;

    UPROPERTY()
    TObjectPtr<AGameCamera> GameCamera;

    UPROPERTY()
    TObjectPtr<AOutfieldPlayerController> PlayerController;

    UPROPERTY(VisibleAnywhere, Category = "SoccerGame|Home Team", meta = (DisplayAfter = "HomeTeamTag"))
    TObjectPtr<ATeamManager> TeamManagerHome;

    UPROPERTY(VisibleAnywhere, Category = "SoccerGame|Away Team", meta = (DisplayAfter = "AwayTeamTag"))
    TObjectPtr<ATeamManager> TeamManagerAway;

    UPROPERTY(VisibleAnywhere, Category = SoccerGame)
    TObjectPtr<APossessionTimer> PossessionTimer;

    UPROPERTY(EditAnywhere, Category= "SoccerGame|Home Team")
    FGameplayTag HomeTeamTag;

    UPROPERTY(EditAnywhere, Category= "SoccerGame|Away Team")
    FGameplayTag AwayTeamTag;

    UPROPERTY(EditAnywhere, Category = "SoccerGame|Debug")
    FGameModeDefaultConfig DefaultConfig;

    FMatchState MatchState;

    // Events:
    //static FOnGameInit GameInitEvent; // Replaces the virtual GameInit() function...
    static inline FOnMatchInit InitMatchEvent{};
    static inline FOnMatchEvent MatchEvent{};
    static inline FOnMatchPhaseRequested MatchPhaseRequestedEvent{};
    static inline FOnMatchPhaseBegin MatchPhaseBeginEvent{};
	// The number of external parties that need to call
    // NotifyReadyForPlayStateChange() before the next PlayState change will occur.
    static inline int32 NumPhaseChangeDependencies = 0;
    static inline int32 NumPhaseChangeDependenciesLeft = 0;

    // The next play state that the game mode should transition to.
    EMatchPhase RequestedPhase;
    EMatchPhase CurrentPhase;
    int32 MaxTeamSize = AProjectSoccerGameMode::kMaxOutfielderCount;

public:
	AProjectSoccerGameMode() = default;

    // Transitioning Match Phases
    void RequestMatchPhaseTransition(const EMatchPhase phase);
    void NotifyReadyForNextPlayState();

    // Updating Match State
    void GoalScored(const FGoalScoredInfo& scoredInfo);
    void BallPossessionChanged(TObjectPtr<AOutfieldCharacter> pOutfielder);
    // void SetFieldState(const FFieldState& fieldState);

    // TODO: Player/Outfielder Management
    // void AddOutfielder();
    // void AddOutfielder();
    // void AddPlayer();
    // void AddPlayer();

    // Events
    static FOnMatchInit& OnMatchInit();
    static FOnMatchEvent& OnMatchEvent();
    static FOnMatchPhaseBegin& OnMatchPhaseBegin();
    static void RemoveOnRequestMatchPhaseListener(const UObject* pObject);

    template <typename UClassType>
    static void AddOnRequestMatchPhaseListener(UClassType* pObject, void(UClassType::*)(const TObjectPtr<AProjectSoccerGameMode>&, EMatchPhase));

    template <typename UClassType>
    static void AddOnRequestMatchPhaseListener(UClassType* pObject, void(UClassType::*)(const TObjectPtr<AProjectSoccerGameMode>&, EMatchPhase) const);

    // Getters
    const FMatchState& GetMatchState() const { return MatchState; }
	TObjectPtr<ATeamManager> GetTeamManager(FGameplayTag teamTag) const;
    TObjectPtr<ASoccerBall> GetSoccerBall() const { return SoccerBall; }
    TObjectPtr<AGameCamera> GetGameCamera() const { return GameCamera; }
    TObjectPtr<APossessionTimer> GetPossessionTimer() const { return PossessionTimer; }
    EMatchPhase GetCurrentMatchPhase() const { return CurrentPhase; }
    EMatchPhase GetRequestedMatchPhase() const { return RequestedPhase; }
    FGameplayTag GetHomeTag() const { return HomeTeamTag; }
    FGameplayTag GetAwayTag() const { return AwayTeamTag; }
    int32 GetMaxTeamSize() const { return MaxTeamSize; }

private:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //virtual void ResetLevel() override;

    void SpawnTeams(const UWorld* pWorld);
    void PushMatchEvent(const EMatchEvent type) const;
    void OnGoalScored(FGameplayTag teamTag);
    void SetMatchPhase(const EMatchPhase phase);
    FGameFormationSet CreateRuntimeFormations() const;
    void OnMatchPhaseBeginCallback(const EMatchPhase phase);

#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Add a member function listener to the MatchPhaseRequested Event.
///     @note : By adding a listener to this event, the Object must call <code>NotifyReadyForNextPlayState()</code>
///             when it is ready for the PlayState to change, regardless if they care about that particular state.
///		@tparam UClassType : UObject class Type.
///		@param pObject : Ptr to the Object (generally passing in the 'this' pointer).
///		@param pFunction : Member function to call when the event is broadcast.
//----------------------------------------------------------------------------------------------------
template <typename UClassType>
void AProjectSoccerGameMode::AddOnRequestMatchPhaseListener(UClassType* pObject, void(UClassType::* pFunction)(const TObjectPtr<AProjectSoccerGameMode>&, EMatchPhase))
{
    ++NumPhaseChangeDependencies;
    MatchPhaseRequestedEvent.AddUObject(pObject, pFunction);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Add a member function listener to the MatchPhaseRequested Event.
///     @note : By adding a listener to this event, the Object must call <code>NotifyReadyForNextPlayState()</code>
///             when it is ready for the PlayState to change, regardless if they care about that particular state.
///		@tparam UClassType : UObject class Type.
///		@param pObject : Ptr to the Object (generally passing in the 'this' pointer).
///		@param pFunction : Member function to call when the event is broadcast.
//----------------------------------------------------------------------------------------------------
template <typename UClassType>
void AProjectSoccerGameMode::AddOnRequestMatchPhaseListener(UClassType* pObject, void(UClassType::* pFunction)(const TObjectPtr<AProjectSoccerGameMode>&, EMatchPhase) const)
{
    ++NumPhaseChangeDependencies;
    MatchPhaseRequestedEvent.AddUObject(pObject, pFunction);
}
