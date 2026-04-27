// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/Entities/Outfielder/OutfieldAIController.h"

#include "OutfieldCharacter.h"
#include "OutfieldPlayerController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectSoccer/Entities/SoccerBall.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"
#include "ProjectSoccer/GameMode/ProjectSoccerGameMode.h"

AOutfieldAIController::AOutfieldAIController()
{
    PrimaryActorTick.bCanEverTick = true;

	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>("Blackboard");
}

void AOutfieldAIController::BeginPlay()
{
    Super::BeginPlay();

    AProjectSoccerGameMode::OnMatchInit().AddUObject(this, &AOutfieldAIController::OnMatchInit);
    AProjectSoccerGameMode::OnMatchPhaseBegin().AddUObject(this, &AOutfieldAIController::OnMatchPhaseBegin);

    // Set Key filters.
	SoccerBallKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(AOutfieldAIController, SoccerBallKey), ASoccerBall::StaticClass());
	TeamRoleTypeKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(AOutfieldAIController, TeamRoleTypeKey), StaticEnum<ERoleType>());
	TeamPossessionStateKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(AOutfieldAIController, TeamPossessionStateKey), StaticEnum<ETeamPossessionState>());
	FormationPositionKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(AOutfieldAIController, FormationPositionKey));
}

void AOutfieldAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    AProjectSoccerGameMode::OnMatchInit().RemoveAll(this);
    AProjectSoccerGameMode::OnMatchPhaseBegin().RemoveAll(this);
}

void AOutfieldAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

    if (ControlledCharacter != nullptr)
    {
        // Move the Character by the input set in the Blackboard.
		const FVector movementVector = GetBlackboardComponent()->GetValueAsVector(MovementVectorKey.SelectedKeyName);
        const FVector2D movementVector2D(movementVector.X, movementVector.Y);
        ControlledCharacter->Move(movementVector2D);
    }
}

#if WITH_EDITOR
void AOutfieldAIController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
    const FName propertyName = PropertyChangedEvent.GetMemberPropertyName();

    if (propertyName == GET_MEMBER_NAME_CHECKED(AOutfieldAIController, InitialBlackboard))
    {
        OnBlackboardOwnerChanged.Broadcast(this, InitialBlackboard);
    }
}
#endif

void AOutfieldAIController::InitializeBlackboardData()
{
	UseBlackboard(InitialBlackboard, BlackboardComponent);
}

void AOutfieldAIController::SetTeamManagerAndBall(TObjectPtr<ATeamManager> pTeamManager, TObjectPtr<ASoccerBall> pSoccerBall)
{
	TeamManager = pTeamManager;

    InitializeBlackboardData();
    BlackboardComponent->SetValueAsObject(SoccerBallKey.SelectedKeyName, pSoccerBall);

    // TODO: Set the Ball radius.
	//BlackboardComponent->SetValueAsFloat(BallTraceRadiusKey.SelectedKeyName, pSoccerBall->)
}

void AOutfieldAIController::OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode)
{
	GameMode = pGameMode;
}

void AOutfieldAIController::OnMatchPhaseBegin(const EMatchPhase phase)
{
	switch (phase)
    {
		case EMatchPhase::Kickoff:
	    case EMatchPhase::PostMatch:
		{
			ControlledCharacter = nullptr;
			UnPossess();
            break;
		}

    	default: break;
    }
}

void AOutfieldAIController::SetStartOutfielder(const FOutfielderId id)
{
    StartPlayerId = id;
}

void AOutfieldAIController::SetControlledOutfielder(const FOutfielderId id)
{
    ControlledPlayerId = id;
    
    check(GameMode != nullptr);
    if (GameMode->GetCurrentMatchPhase() == EMatchPhase::Playing)
    {
	    const auto pOutfielder = TeamManager->GetOutfielder(id);
        Possess(pOutfielder);
    }
}

void AOutfieldAIController::RepossessStartOutfielder()
{
	const auto pOutfielder = TeamManager->GetOutfielder(StartPlayerId);
	Possess(pOutfielder);

    FVector position;
	FRotator rotation;
	TeamManager->GetRespawnPositionAndRotation(StartPlayerId, position, rotation);
    SetFormationPosition(position);
    
    RunBehaviorTree(BehaviorTree);
}

void AOutfieldAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

    ControlledCharacter = Cast<AOutfieldCharacter>(InPawn);
    if (!ControlledCharacter)
    {
	    UE_LOG(LogOutfieldCharacter, Error, TEXT("Failed to cast Pawn to OutFieldCharacter!"));
    }

    else
    {
        // Set our id.
	    ControlledPlayerId = ControlledCharacter->GetOutfielderId();
        ControlledCharacter->SetControlledIconCPU();

        // Register our tick.
        if (!PrimaryActorTick.IsTickFunctionRegistered() && ControlledCharacter)
        {
			PrimaryActorTick.RegisterTickFunction(GetWorld()->GetCurrentLevel());
        }
    }

    RunBehaviorTree(BehaviorTree);
}

void AOutfieldAIController::OnUnPossess()
{
	Super::OnUnPossess();
    ControlledCharacter = nullptr;

    // Unregister our tick.
    if (PrimaryActorTick.IsTickFunctionRegistered())
    {
        PrimaryActorTick.UnRegisterTickFunction();
    }
}

void AOutfieldAIController::SetPossessionTime(const float time) const
{
	BlackboardComponent->SetValueAsFloat(PossessionTimeKey.SelectedKeyName, time);
}

void AOutfieldAIController::SetFormationPosition(const FVector position) const
{
    BlackboardComponent->SetValueAsVector(FormationPositionKey.SelectedKeyName, position);
}

void AOutfieldAIController::SetTeamRole(const ERoleType role) const
{
    BlackboardComponent->SetValueAsEnum(TeamRoleTypeKey.SelectedKeyName, static_cast<uint8>(role));
}

void AOutfieldAIController::SetTeamPossessionState(const ETeamPossessionState possessionState) const
{
	BlackboardComponent->SetValueAsEnum(TeamPossessionStateKey.SelectedKeyName, static_cast<uint8>(possessionState));
}
