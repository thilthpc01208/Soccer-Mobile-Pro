// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Formation.h"
#include "TeamData.h"
#include "TeamRoles.h"
#include "GameFramework/Actor.h"
#include "ProjectSoccer/Entities/Field/FieldState.h"
#include "ProjectSoccer/Entities/Outfielder/OutFielderId.h"
#include "ProjectSoccer/GameMode/MatchEntityInterface.h"
#include "TeamManager.generated.h"

class APossessionTimer;
class UTeamRoleDecision;
DECLARE_LOG_CATEGORY_EXTERN(LogTeamManager, Log, All);
DECLARE_EVENT(ATeamManager, FFormationPositionAssigned);

class ASoccerBall;
class AOutfieldAIController;
class AOutfieldPlayerController;
class UBlackboardData;

UENUM()
enum ETeamPossessionState
{
	Neutral,            // Neither team has possession of the Ball.
    InPossession,       // The Team has possession of the Ball.
    OutOfPossession,    // The Team does not have possession of the Ball.
};

USTRUCT(BlueprintType)
struct FTeamRoleConfig
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TObjectPtr<UTeamRoleDecision> RoleDecision;

private:
    UPROPERTY(EditAnywhere, meta = (ClampMin = 0))
    int32 NumAllowedInRole = 1;

    UPROPERTY(VisibleAnywhere)
    int32 CurrentInRole = 0;

public:
    void AssignPlayerToRole() { ++CurrentInRole; }
    bool RoleQuotaFull() const { return CurrentInRole >= NumAllowedInRole; }
    void Reset() { CurrentInRole = 0; }
};

USTRUCT(BlueprintType)
struct FPossessionStateConfig
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TMap<ERoleType, FTeamRoleConfig> TeamRoles;

    void ResetRoleAssignments();
};

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : The Team Manager is responsible for creating a Team and managing the runtime roles
///     of the AI Players by examining the current FieldState. 
//----------------------------------------------------------------------------------------------------
UCLASS(BlueprintType)
class PROJECTSOCCER_API ATeamManager : public AActor, public IMatchEntityInterface
{
	GENERATED_BODY()

public:
    using TPlayerArray = TArray<TObjectPtr<AOutfieldPlayerController>>;
    using TAIArray = TArray<TObjectPtr<AOutfieldAIController>>;

private:
    UPROPERTY(EditAnywhere, Category = "Team")
    TMap<TEnumAsByte<ETeamPossessionState>, FPossessionStateConfig> PossessionConfigs;

    UPROPERTY(VisibleAnywhere, Category = "Team|Runtime")
    FRuntimeTeamData RuntimeTeamData;

    UPROPERTY(VisibleAnywhere, Category = "Team|Runtime")
    TObjectPtr<UGameFormation> CurrentFormation;

	//----------------------------------------------------------------------------------------------------
	///		@brief : Outfielders that are part of this Team.
	//----------------------------------------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, Category = "Team|Runtime")
    TArray<TObjectPtr<AOutfieldCharacter>> Outfielders;

    UPROPERTY(VisibleAnywhere, Category = "Team|Runtime")
    TArray<TObjectPtr<AOutfieldAIController>> AIControllers;

    UPROPERTY(VisibleAnywhere, Category = "Team|Runtime")
    TEnumAsByte<ETeamPossessionState> PossessionState;

    UPROPERTY(EditInstanceOnly, Category = Team)
    FGameplayTag TeamTag;

    UPROPERTY(EditAnywhere, Category = "Team|Debug")
    FColor ActivePositionColor;

    UPROPERTY(EditAnywhere, Category = "Team|Debug")
    FColor UnassignedPositionColor;

    UPROPERTY()
    TObjectPtr<APossessionTimer> PossessionTimer;

    UPROPERTY()
    TObjectPtr<ASoccerBall> SoccerBall;

    UPROPERTY()
    TObjectPtr<AProjectSoccerGameMode> GameMode;

    FFormationPositionAssigned FormationPositionAssignedEvent;

    UFieldState FieldState{};

    UPROPERTY(VisibleAnywhere, Category = "Team|Runtime")
    bool bScoredOn = false;

    // TODO: (Stretch goal) Some sort of GameAnalysis data.

public:
	ATeamManager();

	virtual void Tick(float DeltaTime) override;
    void InitTeam(const TObjectPtr<AProjectSoccerGameMode>& pGameMode, const FRuntimeTeamData& teamData);

    bool TryAddOutfielder();
    bool TryAddPlayer(const TObjectPtr<AOutfieldPlayerController>& pPlayerController);
    bool TryRemovePlayer(const TObjectPtr<AOutfieldPlayerController>& pPlayerController);

    void SetFieldState(const UFieldState& state) { FieldState = state; }

    UFUNCTION(BlueprintCallable)
	const UGameFormation* GetFormation(EFormationType key) const;

    UFUNCTION(BlueprintCallable)
    const UGameFormation* GetCurrentFormation() const { return CurrentFormation; }

    UFUNCTION(BlueprintCallable)
    AActor* GetPlayerInPossessionOrBall() const;

    UFUNCTION(BlueprintCallable)
    AActor* GetPlayerInPossession() const;

    // TODO: Move to the Field Manager
    UFUNCTION(BlueprintCallable)
    AGoalTrigger* GetOpponentGoal() const { return RuntimeTeamData.OpponentGoal; }

    // TODO: Move to the Field Manager
    UFUNCTION(BlueprintCallable)
    AGoalTrigger* GetTeamGoal() const { return RuntimeTeamData.TeamGoal; }

    UFUNCTION(BlueprintCallable)
    const TArray<AOutfieldCharacter*>& GetOutfielders() const { return Outfielders; }
    const TArray<TObjectPtr<AOutfieldCharacter>>& GetObjectPtrOutfielders() const { return Outfielders; }

    const FRuntimeTeamData& GetTeamData() const { return RuntimeTeamData; }
    TObjectPtr<ASoccerBall> GetBall() const { return SoccerBall; }
    TObjectPtr<AOutfieldCharacter> GetOutfielder(const FOutfielderId id) const;
    TObjectPtr<AOutfieldCharacter> GetOutfielder(const int32 playerIndex) const;

    // TODO: Move the positional queries to the Field Manager.
    TObjectPtr<AOutfieldCharacter> GetClosestTeammate(const FOutfielderId id) const;
    TObjectPtr<AOutfieldCharacter> GetClosestTeammateToLocation(const AOutfieldCharacter* pOutfielder, const FVector& location) const;
    TObjectPtr<AOutfieldCharacter> GetClosestOutfielderToLocation(const FVector location) const;
    bool LocationInTeamHalf(const FVector position) const;
    bool OutfielderInTeamHalf(const AOutfieldCharacter* pOutfielder) const;

    // TODO: Should I have this here? This is the Quick way to get things running
    // again for AI systems like Steering and Influence Tests in EQS.
    const UFieldState& GetFieldState() const { return FieldState; }
    void GetRespawnPositionAndRotation(const FOutfielderId id, FVector& position, FRotator& rotation) const;
    float GetInfluenceValue() const { return RuntimeTeamData.InfluenceValue; }
    FGameplayTag GetTeamTag() const { return TeamTag; }
    FFormationPositionAssigned& OnFormationPositionAssigned() { return FormationPositionAssignedEvent; }
    ETeamPossessionState GetPossessionState() const { return PossessionState; }
    FColor GetActivePositionColor() const { return ActivePositionColor; }
    FColor GetUnassignedPositionColor() const { return UnassignedPositionColor; }

private:
	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode) override;
    virtual void OnMatchPhaseBegin(const EMatchPhase phase) override;
    void OnMatchEvent(const EMatchEvent event, const FMatchState& matchState);
    void OnRequestMatchPhase(const TObjectPtr<AProjectSoccerGameMode>& pGameMode, const EMatchPhase requestedPhase);

    void InitFormations() const;
    void UpdateFormationAndRoles();
    void AssignTeamRoles();
	void UpdatePossessionTime() const;
    ERoleType DetermineRoleForOutfielder(const AOutfieldCharacter* pOutfielder);
	void AssignFormationPositions();
    void SortFormationPositions(TArray<FVector>& positions) const;
    void ResetTeam();
    void ResetAllRoles();
    void ResetAllFormations() const;
    void OnTeamPossessionChanged(FGameplayTag teamInPossessionTag);
};