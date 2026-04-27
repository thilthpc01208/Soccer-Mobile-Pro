// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "OutfielderId.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"
#include "BehaviorTree/BlackboardAssetProvider.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectSoccer/GameMode/MatchEntityInterface.h"
#include "ProjectSoccer/Entities/Team/TeamRoles.h"
#include "ProjectSoccer/AI/EQS/EnvQueryGenerator_Formation.h"
#include "OutfieldAIController.generated.h"

class ASoccerBall;
class ATeamManager;

UCLASS()
class PROJECTSOCCER_API AOutfieldAIController : public AAIController, public IMatchEntityInterface, public IBlackboardAssetProvider
{
	GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Outfielder|Blackboard")
    UBehaviorTree* BehaviorTree = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Outfielder|Blackboard")
    UBlackboardData* InitialBlackboard = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Outfielder|Blackboard")
    FBlackboardKeySelector SoccerBallKey;

    UPROPERTY(EditDefaultsOnly, Category = "Outfielder|Blackboard")
    FBlackboardKeySelector TeamRoleTypeKey;

    UPROPERTY(EditDefaultsOnly, Category = "Outfielder|Blackboard")
    FBlackboardKeySelector FormationPositionKey;

    UPROPERTY(EditDefaultsOnly, Category = "Outfielder|Blackboard")
    FBlackboardKeySelector TeamPossessionStateKey;

    UPROPERTY(EditDefaultsOnly, Category = "Outfielder|Blackboard")
    FBlackboardKeySelector MovementVectorKey;

    UPROPERTY(EditDefaultsOnly, Category = "Outfielder|Blackboard")
    FBlackboardKeySelector PossessionTimeKey;

    UPROPERTY(EditDefaultsOnly, Category = "Outfielder|Blackboard")
    FBlackboardKeySelector BallTraceRadiusKey;

    UPROPERTY()
    UBlackboardComponent* BlackboardComponent = nullptr;

    UPROPERTY(VisibleAnywhere, Category = Outfielder)
    FOutfielderId StartPlayerId;

    UPROPERTY(VisibleAnywhere, Category= Outfielder)
    FOutfielderId ControlledPlayerId;

    UPROPERTY(VisibleAnywhere, Category= Outfielder)
    TObjectPtr<class AOutfieldCharacter> ControlledCharacter;

    UPROPERTY()
    TObjectPtr<ATeamManager> TeamManager;

    UPROPERTY()
    TObjectPtr<AProjectSoccerGameMode> GameMode;

public:
    AOutfieldAIController();

    // MatchEntityInterface
    virtual void OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode) override;
    virtual void OnMatchPhaseBegin(const EMatchPhase phase) override;

    void SetTeamManagerAndBall(TObjectPtr<ATeamManager> pTeamManager, TObjectPtr<ASoccerBall> pSoccerBall);
    void RepossessStartOutfielder();

    void SetPossessionTime(const float time) const;
    void SetStartOutfielder(const FOutfielderId id);
    void SetControlledOutfielder(const FOutfielderId id);
    void SetFormationPosition(const FVector position) const;
    void SetTeamRole(const ERoleType role) const;
    void SetTeamPossessionState(const ETeamPossessionState possessionState) const;

    UFUNCTION(BlueprintCallable)
    ATeamManager* GetTeamManager() { return TeamManager; }
    AOutfieldCharacter* GetControlledOutfielder() const { return ControlledCharacter; }
    FOutfielderId GetLastControlledPlayerId() const { return ControlledPlayerId; }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    void InitializeBlackboardData();
    virtual UBlackboardData* GetBlackboardAsset() const override { return InitialBlackboard; }
};

