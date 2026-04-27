// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectSoccerGameMode.h"

#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "ProjectSoccer/AI/Team/PossessionTimer.h"
#include "ProjectSoccer/Entities/GameCamera.h"
#include "ProjectSoccer/Entities/Field/GoalTrigger.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldPlayerController.h"
#include "ProjectSoccer/Entities/SoccerBall.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"

DEFINE_LOG_CATEGORY(LogSoccerGameMode);

static FString GetMatchPhaseName(const EMatchPhase phase)
{
    switch (phase)
    {
    	case EMatchPhase::Null: return TEXT("Null");
		case EMatchPhase::PreMatch: return TEXT("PreMatch");
		case EMatchPhase::Kickoff: return TEXT("Kickoff");
		case EMatchPhase::Playing: return TEXT("Playing");
		case EMatchPhase::PostMatch: return TEXT("PostMatch");
		default: return TEXT("Unknown");
    }
}

void AProjectSoccerGameMode::BeginPlay()
{
	Super::BeginPlay();
    
    // TEMP: GameMode is going to facilitate the transition
    // between Kickoff and Playing.
    OnMatchPhaseBegin().AddUObject(this, &AProjectSoccerGameMode::OnMatchPhaseBeginCallback);

    auto* pWorld = GetWorld();

    // Spawn Ball, and possession timer.
    SoccerBall = pWorld->SpawnActor<ASoccerBall>(SoccerBallClass);
    PossessionTimer = pWorld->SpawnActor<APossessionTimer>(FVector{ 0, 0, 50.0}, FRotator::ZeroRotator);

    // Get the Game Camera in the Level.
    auto* pCameraActor = UGameplayStatics::GetActorOfClass(pWorld, AGameCamera::StaticClass());
    check(pCameraActor != nullptr);
    GameCamera = Cast<AGameCamera>(pCameraActor);

    // Spawn Teams
    SpawnTeams(pWorld);

    // Initialize the Match Entities.
    InitMatchEvent.Broadcast(this);

    //float value;
    //UE_LOG(LogSoccerGameMode, Warning, TEXT("Match Initialized! %"), value);

    // Set our MatchPhase to PreMatch.
    // Request the Kickoff Phase.
    SetMatchPhase(EMatchPhase::PreMatch);
    RequestMatchPhaseTransition(EMatchPhase::Kickoff);

#if !UE_BUILD_SHIPPING
    // [TODO]: This could probably be in another location, like in the Constructor of the
    // DebugViewRegistry.
    Internal::DebugViewRegistry::Get().RegisterDebugViews(DebugViewsTable);
    // Refresh the Active View, if there was one enabled in the last session.
    Internal::DebugViewRegistry::Get().RefreshActiveView();
#endif
}

void AProjectSoccerGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    // Clear the DebugViews array.
    Internal::DebugViewRegistry::Get().Clear();
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      I could have separate functions for Pause(), Resume(), etc, that then call this private func.
//      I may have an issue making sure that a Phase Transition has finished before other requests come in.
//		
///		@brief : Request a change to the MatchPlayState.
///		@param phase : State to change to.
//----------------------------------------------------------------------------------------------------
void AProjectSoccerGameMode::RequestMatchPhaseTransition(const EMatchPhase phase)
{
    // If no state change, or same request return.
    if (CurrentPhase == phase || RequestedPhase == phase)
        return;

    // If there are no listeners, then immediately update the PlayState.
    if (!MatchPhaseRequestedEvent.IsBound())
    {
	    SetMatchPhase(phase);
        return;
    }

    // Put in the Request, and wait for the listeners to call NotifyReadyForNextPlayState().
    // TODO: Should this be per Phase type?
    NumPhaseChangeDependenciesLeft = NumPhaseChangeDependencies;
    RequestedPhase = phase;
    MatchPhaseRequestedEvent.Broadcast(this, phase);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Notify the GameMode that the caller is ready for the PlayState to change.
///              This should only be called by listeners to the OnMatchPlayStateRequested Event.
//----------------------------------------------------------------------------------------------------
void AProjectSoccerGameMode::NotifyReadyForNextPlayState()
{
    --NumPhaseChangeDependenciesLeft;

    // If there are no more external dependencies, then update the PlayState.
    if (NumPhaseChangeDependenciesLeft <= 0)
    {
        NumPhaseChangeDependenciesLeft = 0;
        SetMatchPhase(RequestedPhase);
    }
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      TODO: I should take in more information to build the MatchStats in the future.
//		
///		@brief : Notify the GameMode that a Goal has been scored.
///		@param scoredInfo : Info about how the goal was scored and who did it.
//----------------------------------------------------------------------------------------------------
void AProjectSoccerGameMode::GoalScored(const FGoalScoredInfo& scoredInfo)
{
    check(scoredInfo.ScoringTeam == HomeTeamTag || scoredInfo.ScoringTeam == AwayTeamTag);

    if (scoredInfo.ScoringTeam == HomeTeamTag)
    {
        MatchState.HomeTeamScore += 1;
    }

    else if (scoredInfo.ScoringTeam == AwayTeamTag)
    {
        MatchState.AwayTeamScore += 1;
    }

    // Add the record of the goal in the match state.
    MatchState.GoalsScored.Add(scoredInfo);

    PushMatchEvent(EMatchEvent::GoalScored);
    RequestMatchPhaseTransition(EMatchPhase::Kickoff);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Update the MatchState with new Possession Information.
///		@param pOutfielder : Outfielder with possession of the Ball. Can be nullptr if the ball is not
///             in possession by either team.
//----------------------------------------------------------------------------------------------------
void AProjectSoccerGameMode::BallPossessionChanged(TObjectPtr<AOutfieldCharacter> pOutfielder)
{
    if (pOutfielder == MatchState.OutfielderInPossession)
        return;

    // Get the TeamTag of the current outfielder in possession, or null.
    const FGameplayTag current = MatchState.OutfielderInPossession != nullptr ? MatchState.OutfielderInPossession->GetTeamTag() : FGameplayTag::EmptyTag;

    // Notify that a new Outfielder is in possession.
    MatchState.OutfielderInPossession = pOutfielder;
    PushMatchEvent(EMatchEvent::OutfielderPossessionChange);

    // Check to see if the Team in possession has changed.
    const FGameplayTag newTag = pOutfielder != nullptr ? pOutfielder->GetTeamTag() : FGameplayTag::EmptyTag;

    if (current != newTag)
		PushMatchEvent(EMatchEvent::TeamPossessionChange);
}

//----------------------------------------------------------------------------------------------------
///		@brief : The OnMatchInit Event is called during the GameMode's BeginPlay to initialize all the Match Entities.
//----------------------------------------------------------------------------------------------------
FOnMatchInit& AProjectSoccerGameMode::OnMatchInit()
{
    return InitMatchEvent;
}

//----------------------------------------------------------------------------------------------------
///		@brief : A Match Event occurs anytime the MatchState changes.
//----------------------------------------------------------------------------------------------------
FOnMatchEvent& AProjectSoccerGameMode::OnMatchEvent()
{
    return MatchEvent;
}

//----------------------------------------------------------------------------------------------------
///		@brief : The MatchPhaseBegin Event occurs when the MatchPhase officially changes.
//----------------------------------------------------------------------------------------------------
FOnMatchPhaseBegin& AProjectSoccerGameMode::OnMatchPhaseBegin()
{
    return MatchPhaseBeginEvent;
}

//----------------------------------------------------------------------------------------------------
///		@brief : Remove a listener from the OnMatchPlayStateRequested Event.
//----------------------------------------------------------------------------------------------------
void AProjectSoccerGameMode::RemoveOnRequestMatchPhaseListener(const UObject* pObject)
{
    MatchPhaseRequestedEvent.RemoveAll(pObject);
    --NumPhaseChangeDependencies;
}

TObjectPtr<ATeamManager> AProjectSoccerGameMode::GetTeamManager(FGameplayTag teamTag) const
{
    if (teamTag == TeamManagerHome->GetTeamTag())
    {
	    return TeamManagerHome;
    }

    if (teamTag == TeamManagerAway->GetTeamTag())
    {
	    return TeamManagerAway;
    }

    return nullptr;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      TODO: Right now, I am just creating some default Runtime Data. I plan on taking the
//         data from the GameState/GameInstance in the future. That data will be set in a previous level, in the
//         Menus.
//		
///		@brief : Spawn the Teams based on the passed in TeamData.
///		@param pWorld : World to spawn the teams in.
//----------------------------------------------------------------------------------------------------
void AProjectSoccerGameMode::SpawnTeams(const UWorld* pWorld)
{
    // TEMP: Grab the TeamManagers from the Level.
    // - I should probably just spawn these. They were in the level because it had "Field" Responsibilities.
    TArray<AActor*> teamManagers; 
    UGameplayStatics::GetAllActorsOfClass(pWorld, ATeamManager::StaticClass(), teamManagers);
    if (teamManagers.Num() != 2)
    {
        UE_LOG(LogSoccerGameMode, Error, TEXT("Failed to setup game! There were: %d teamManagers in the Level! There needs to be 2!"), teamManagers.Num());
        return;
    }

    // Set the Home team and Away team to match the TeamManager.
    ATeamManager* pFirstTeamManagerFound = Cast<ATeamManager>(teamManagers[0]);
    const bool bHomeTeamFirst = pFirstTeamManagerFound->GetTeamTag() == HomeTeamTag;
    TeamManagerHome = bHomeTeamFirst? pFirstTeamManagerFound : Cast<ATeamManager>(teamManagers[1]);
    TeamManagerAway = !bHomeTeamFirst? pFirstTeamManagerFound : Cast<ATeamManager>(teamManagers[1]);

    // Grab the Goal Triggers
    TArray<AActor*> goalTriggers;
    UGameplayStatics::GetAllActorsOfClass(pWorld, AGoalTrigger::StaticClass(), goalTriggers);
    if (goalTriggers.Num() != 2)
    {
        UE_LOG(LogSoccerGameMode, Error, TEXT("Failed to setup game! There were: %d goalTriggers in the Level! There needs to be 2!"), goalTriggers.Num());
        return;
    }

    AGoalTrigger* pFirst = Cast<AGoalTrigger>(goalTriggers[0]);
    const bool bHomeGoalFirst = pFirst->GetScoringTeamTag() == AwayTeamTag;
    AGoalTrigger* pHomeGoal = bHomeGoalFirst ? pFirst : Cast<AGoalTrigger>(goalTriggers[1]);
    AGoalTrigger* pAwayGoal = !bHomeGoalFirst ? pFirst : Cast<AGoalTrigger>(goalTriggers[1]);

    // TEMP: Grab the first PlayerController, and set it on the Home Team.
    // [TODO]: I should have a function that handles the PlayerController
    //         registration.
    auto* pPlayerController = pWorld->GetFirstPlayerController();
    auto* pOutFielderPlayerController = Cast<AOutfieldPlayerController>(pPlayerController);
    pPlayerController->SetShowMouseCursor(false);
    pPlayerController->SetInputMode(FInputModeGameOnly());

    // TEMP: Create the Runtime Team Data.
    // Set up the Home Team.
    FRuntimeTeamData teamData;
    teamData.TeamTag = HomeTeamTag;
    teamData.DefaultPlayerClass = DefaultConfig.OutfielderClass;
    teamData.TeamSize = FMath::Max(DefaultConfig.HomeTeamSize, 1);
    teamData.Players.Add(pOutFielderPlayerController);
    teamData.TeamMaterial = DefaultConfig.HomeMaterial;
    teamData.InfluenceValue = 1.0;
    teamData.TeamGoal = pHomeGoal;
    teamData.OpponentGoal = pAwayGoal;
    teamData.Formations = CreateRuntimeFormations();
    TeamManagerHome->InitTeam(this, teamData);

    // Set up the Away Team.
    teamData.Players.Empty();
    teamData.DefaultPlayerClass = DefaultConfig.OutfielderClass;
    teamData.TeamSize = DefaultConfig.AwayTeamSize;
    teamData.InfluenceValue = -1.0;
    teamData.TeamTag = AwayTeamTag;
    teamData.TeamMaterial = DefaultConfig.AwayMaterial;
    teamData.Formations = CreateRuntimeFormations();
    teamData.TeamGoal = pAwayGoal;
    teamData.OpponentGoal = pHomeGoal;
    TeamManagerAway->InitTeam(this, teamData);
}

void AProjectSoccerGameMode::PushMatchEvent(const EMatchEvent type) const
{
    // Pass the MatchState?
    MatchEvent.Broadcast(type, MatchState);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Create the Runtime Formations from the GameMode's set formations.
//----------------------------------------------------------------------------------------------------
FGameFormationSet AProjectSoccerGameMode::CreateRuntimeFormations() const
{
    FGameFormationSet formations{};
    formations.Neutral = NewObject<UGameFormation>();
    formations.Neutral->SetFormationData(FormationSet->Neutral);

    formations.ScoredOn = NewObject<UGameFormation>();
    formations.ScoredOn->SetFormationData(FormationSet->ScoredOn);

    formations.Attack = NewObject<UGameFormation>();
    formations.Attack->SetFormationData(FormationSet->Attack);

    formations.Defense = NewObject<UGameFormation>();
    formations.Defense->SetFormationData(FormationSet->Defense);

    return formations;
}

void AProjectSoccerGameMode::OnGoalScored(FGameplayTag teamTag)
{
	if (teamTag == TeamManagerHome->GetTeamTag())
	{
		MatchState.AwayTeamScore += 1;		
	}

    else if (teamTag == TeamManagerAway->GetTeamTag())
    {
        MatchState.HomeTeamScore += 1;
    }

    //GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Resetting level on goal scored."));
    // TODO: Some sort of Cinematic stuff.
    // For now, just reset the level.
    ResetLevel();
}

//----------------------------------------------------------------------------------------------------
///		@brief : Updates the Current Phase and broadcasts the MatchPhaseBegin Event.
//----------------------------------------------------------------------------------------------------
void AProjectSoccerGameMode::SetMatchPhase(const EMatchPhase phase)
{
	NumPhaseChangeDependenciesLeft = 0;
    CurrentPhase = phase;
    RequestedPhase = EMatchPhase::Null;

    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Setting Match Phase: %s"), *GetMatchPhaseName(phase)));
    
    MatchPhaseBeginEvent.Broadcast(CurrentPhase);
}

void AProjectSoccerGameMode::OnMatchPhaseBeginCallback(const EMatchPhase phase)
{
    switch (phase)
    {
        // TEMP: When in at the Kickoff, transition to Playing.
        //      This will be handled by certain UI.
    	case EMatchPhase::Kickoff:
    	{
    		RequestMatchPhaseTransition(EMatchPhase::Playing);
            break;
    	}

    	default: break;
    }
}

// [TODO]: Move this functionality to a separate class, preferably the one that runs the DebugView UI.
// - Is it a HUD?
#if WITH_EDITOR
void AProjectSoccerGameMode::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
    /*Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName propertyName = PropertyChangedEvent.GetPropertyName();
    if (propertyName == GET_MEMBER_NAME_CHECKED(AProjectSoccerGameMode, ActiveDebugViewIndex))
    {
        if (DebugViewsTable == nullptr)
            return;

        TArray<FDebugView*> views;
        DebugViewsTable->GetAllRows("Test", views);

        if (ActiveDebugViewIndex < 0 || ActiveDebugViewIndex >= views.Num())
        {
            ActiveDebugViewIndex = -1;
            Internal::DebugViewRegistry::Get().ClearActiveView();
            return;
        }

        const auto* pView = views[ActiveDebugViewIndex];
        Internal::DebugViewRegistry::Get().SetActiveView(pView);
    }*/
}
#endif
