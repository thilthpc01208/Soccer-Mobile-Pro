// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/Entities/Team/TeamManager.h"

#include "TeamRoles.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectSoccer/AI/Team/PossessionTimer.h"
#include "ProjectSoccer/AI/Team/TeamRoleDecision.h"
#include "ProjectSoccer/Entities/GameCamera.h"
#include "ProjectSoccer/Entities/SoccerBall.h"
#include "ProjectSoccer/Entities/Field/GoalTrigger.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldAIController.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldPlayerController.h"
#include "ProjectSoccer/GameMode/ProjectSoccerGameMode.h"

DEFINE_LOG_CATEGORY(LogTeamManager);

void FPossessionStateConfig::ResetRoleAssignments()
{
	for (auto& [roleType, roleConfig] : TeamRoles)
	{
		roleConfig.Reset();
	}
}

// Sets default values
ATeamManager::ATeamManager()
{
    PossessionState = Neutral;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.2f;

    PossessionConfigs.Add(ETeamPossessionState::Neutral, FPossessionStateConfig{});
    PossessionConfigs.Add(ETeamPossessionState::InPossession, FPossessionStateConfig{});
    PossessionConfigs.Add(ETeamPossessionState::OutOfPossession, FPossessionStateConfig{});
}

void ATeamManager::BeginPlay()
{
	Super::BeginPlay();

    // Register for GameMode Events.
    AProjectSoccerGameMode::OnMatchInit().AddUObject(this, &ATeamManager::OnMatchInit);
    AProjectSoccerGameMode::OnMatchPhaseBegin().AddUObject(this, &ATeamManager::OnMatchPhaseBegin);
    AProjectSoccerGameMode::OnMatchEvent().AddUObject(this, &ATeamManager::OnMatchEvent);
    AProjectSoccerGameMode::AddOnRequestMatchPhaseListener(this, &ATeamManager::OnRequestMatchPhase);
}

void ATeamManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // Unregister for GameMode Events.
    AProjectSoccerGameMode::OnMatchInit().RemoveAll(this);
    AProjectSoccerGameMode::OnMatchPhaseBegin().RemoveAll(this);
    AProjectSoccerGameMode::OnMatchEvent().RemoveAll(this);
    AProjectSoccerGameMode::RemoveOnRequestMatchPhaseListener(this);
}

//----------------------------------------------------------------------------------------------------
///		@brief : Initialize the Team with the given TeamData. Create the Outfielders for the TeamSize, and
///             assign the PlayerControllers to the Team.
//----------------------------------------------------------------------------------------------------
void ATeamManager::InitTeam(const TObjectPtr<AProjectSoccerGameMode>& pGameMode, const FRuntimeTeamData& teamData)
{
    GameMode = pGameMode;
    RuntimeTeamData = teamData;
    SoccerBall = pGameMode->GetSoccerBall();

    // TODO: This should be on the FieldManager?
    InitFormations();

    // Create the Outfielders
    for (size_t i = 0; i < teamData.TeamSize; ++i)
    {
        TryAddOutfielder();
    }

    TObjectPtr<AGameCamera> pCamera = GameMode->GetGameCamera();
    AActor* pCameraActor = Cast<AActor>(pCamera.Get());

    // Set up the Players
    for (size_t i = 0; i < RuntimeTeamData.Players.Num(); ++i)
    {
	    const auto& pPlayerController = RuntimeTeamData.Players[i];
        //pPlayerController->GameInit(pGameMode);
        pPlayerController->SetTeamManager(this);
	    pPlayerController->SetStartOutfielder(Outfielders[i]->GetOutfielderId());
        pPlayerController->SetViewTarget(Cast<AActor>(pCameraActor));
    }
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Try to add a new Outfielder to this Team. If the maximum number of Outfielders
///            have already been spawned, then this function will return false.
//----------------------------------------------------------------------------------------------------
bool ATeamManager::TryAddOutfielder()
{
	// Create the Outfielder object, at a spawn location.

    // Make sure that we don't exceed the max team size.
    // TODO: GameMode check for the allowed team size?
    if (Outfielders.Num() == GameMode->GetMaxTeamSize())
    {
	    UE_LOG(LogTeamManager, Warning, TEXT("Cannot add more than %d outfielders per team!"), GameMode->GetMaxTeamSize());
        return false;
    }

    const FOutfielderId id{ TeamTag, (Outfielders.Num()) };
    const FVector location = RuntimeTeamData.Formations.Neutral->GetLocationOfPoint(id.OutfielderIndex);
    const FRotator rotation = RuntimeTeamData.Formations.Neutral->GetFormationRotation();

    auto* pWorld = GetWorld();
    auto* pOutfielder = pWorld->SpawnActor<AOutfieldCharacter>(RuntimeTeamData.DefaultPlayerClass, location, rotation);
    pOutfielder->SetOutfielderInfo(FOutfielderCreateInfo{ this, RuntimeTeamData.TeamMaterial, id, false });
    Outfielders.Add(pOutfielder);
    RuntimeTeamData.TeamSize = Outfielders.Num();

    // Spawn an AI Controller for that Outfielder:
    auto* pAIController = pWorld->SpawnActor<AOutfieldAIController>(pOutfielder->AIControllerClass, location, rotation);
    pAIController->SetTeamManagerAndBall(this, SoccerBall);
    pAIController->SetStartOutfielder(pOutfielder->GetOutfielderId());
    pAIController->SetTeamRole(ERoleType::StayInFormation);
    pAIController->SetFormationPosition(location);
    AIControllers.Add(pAIController);

    // If the Match is underway, then we need to call
    // OnMatchInit, then Begin the Current Match Phase.
    const auto matchPhase = GameMode->GetCurrentMatchPhase();

    if (matchPhase != EMatchPhase::Null)
    {
        // Initialize the Outfielder
        pOutfielder->OnMatchInit(GameMode);
        pOutfielder->OnMatchPhaseBegin(matchPhase);

        // Initialize the AI Controller
        pAIController->OnMatchInit(GameMode);
        pAIController->OnMatchPhaseBegin(matchPhase);
    }

    return true;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Try to add a new Player Controller to this Team.
///		@returns : Returns false in the event that the maximum number of players have been reached or
///             if the player is already on the team.
//----------------------------------------------------------------------------------------------------
bool ATeamManager::TryAddPlayer(const TObjectPtr<AOutfieldPlayerController>& pPlayerController)
{
    if (RuntimeTeamData.Players.Num() == RuntimeTeamData.TeamSize)
    {
        UE_LOG(LogTeamManager, Warning, TEXT("Cannot add more than %d players per team!"), RuntimeTeamData.TeamSize);
        return false;
    }

    // Check if the Player is already on the team.
    for (auto pPlayer : RuntimeTeamData.Players)
    {
        if (pPlayer == pPlayerController)
        {
            UE_LOG(LogTeamManager, Warning, TEXT("Player is already on the team!"));
            return false;
        }
    }

    TObjectPtr<AGameCamera> pCamera = GameMode->GetGameCamera();
    AActor* pCameraActor = Cast<AActor>(pCamera.Get());

    // Add the Player to the Team, and assign their Starting Outfielder
    const size_t index = RuntimeTeamData.Players.Add(pPlayerController);
    pPlayerController->SetTeamManager(this);
	pPlayerController->SetStartOutfielder(Outfielders[index]->GetOutfielderId());
    pPlayerController->SetViewTargetWithBlend(pCameraActor);

    // If the Match is underway, then we need to call
    // OnMatchInit, then Begin the Current Match Phase.
    const auto matchPhase = GameMode->GetCurrentMatchPhase();
    if (matchPhase != EMatchPhase::Null)
    {
        pPlayerController->OnMatchInit(GameMode);
        pPlayerController->OnMatchPhaseBegin(matchPhase);

        // Find the next available Outfielder to possess.
        for (auto& pAiController : AIControllers)
        {
            auto* pOutfielder = pAiController->GetControlledOutfielder();
            if (pOutfielder != nullptr)
            {
	            pPlayerController->Possess(pOutfielder);
                break;
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Try to Remove a Player from the team.
///		@param pPlayerController : PlayerController to remove.
///		@returns : False if the Player is null or not on the team.
//----------------------------------------------------------------------------------------------------
bool ATeamManager::TryRemovePlayer(const TObjectPtr<AOutfieldPlayerController>& pPlayerController)
{
    if (pPlayerController == nullptr)
    {
        UE_LOG(LogTeamManager, Warning, TEXT("Tried to remove a null player from the team!"));
        return false;
    }

    for (size_t i = 0; i < RuntimeTeamData.Players.Num(); ++i)
    {
        if (RuntimeTeamData.Players[i] == pPlayerController)
        {
            // Have the AI take control of the Outfielder:
            auto* pAIController = pPlayerController->GetOverwrittenAIController();
            check (pAIController);
            pAIController->Possess(RuntimeTeamData.Players[i]->GetControlledOutfielder());

            // Remove the player from the team.
            RuntimeTeamData.Players.RemoveAt(i);
            return true;
        }
    }

    UE_LOG(LogTeamManager, Warning, TEXT("Tried to remove player that is not on this Team!"));
    return false;
}

void ATeamManager::OnMatchEvent(const EMatchEvent event, const FMatchState& matchState)
{
	switch (event)
	{
		case EMatchEvent::GoalScored:
		{
            const auto& lastGoalInfo = matchState.GoalsScored.Last();
			bScoredOn = lastGoalInfo.ScoringTeam != RuntimeTeamData.TeamTag;
            break;
		}

		case EMatchEvent::TeamPossessionChange:
		{
            OnTeamPossessionChanged(matchState.GetTeamInPossessionTag());
            break;
		}

        default: break;
	}
}

void ATeamManager::OnRequestMatchPhase(const TObjectPtr<AProjectSoccerGameMode>& pGameMode, const EMatchPhase requestedPhase)
{
    switch (requestedPhase)
    {
        // We need to reset our team before we can enter the Playing State.
        case EMatchPhase::Playing:
    	{
            // Reset our formation based on the result:
            // TODO: Get the Scored on Version working.
            CurrentFormation = bScoredOn? RuntimeTeamData.Formations.ScoredOn : RuntimeTeamData.Formations.Neutral;
			//CurrentFormation = RuntimeTeamData.Formations.Neutral; 
			FormationPositionAssignedEvent.Broadcast();
            ResetTeam();

            // Reset the AIControllers' roles to StayInFormation
            for (auto& pAIController : AIControllers)
            {
                pAIController->SetTeamRole(ERoleType::StayInFormation);
            }

            break;
    	}

    	default: break;
    }

    // Notify that we are ready for the state change.
    GameMode->NotifyReadyForNextPlayState();
}

void ATeamManager::OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode)
{
    GameMode = pGameMode;
    PossessionTimer = pGameMode->GetPossessionTimer();
    SoccerBall = pGameMode->GetSoccerBall();
}

void ATeamManager::OnMatchPhaseBegin(const EMatchPhase phase)
{
	switch (phase)
	{
		case EMatchPhase::Playing:
        {
            // Have each AiController repossess their start character
	        for (TObjectPtr<AOutfieldAIController> pAIController : AIControllers)
	        {
		        pAIController->RepossessStartOutfielder();
                
	        }

            // Then, have each PlayerController repossess their start character
            for (TObjectPtr<AOutfieldPlayerController> pPlayerController : RuntimeTeamData.Players)
	        {
		        pPlayerController->RepossessStartOutfielder();
	        }
        }

		default: break;
	}
}

void ATeamManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    AssignTeamRoles();
    UpdatePossessionTime();
}

void ATeamManager::UpdateFormationAndRoles()
{
	AssignFormationPositions();
    AssignTeamRoles();

	FormationPositionAssignedEvent.Broadcast();
}

void ATeamManager::AssignFormationPositions()
{
    if (PossessionState == Neutral)
        return;

    auto& positions = CurrentFormation->GetMutablePositions();
    CurrentFormation->ClearAllAssignments();
    SortFormationPositions(positions);

    // Then for each player, assign the formation position that is closest to them.
    size_t outfieldersToAssign = Outfielders.Num();
    for (size_t positionIndex = 0; positionIndex < positions.Num(); ++positionIndex)
    {
        // TODO: Delete:
        // If the position is already filled, then decrement the outfielders to assign are return
        if (CurrentFormation->GetPositionAssigned(positionIndex))
        {
	        --outfieldersToAssign;
            continue;
        }

        const FVector point = positions[positionIndex];
		int bestOutfielderIndex = -1;
        float closestDistanceSqr = std::numeric_limits<float>::max();

        // For each player left to assign:
        for (size_t outfielderIndex = 0; outfielderIndex < outfieldersToAssign;)
        {
            TObjectPtr<AOutfieldAIController> pAiController = AIControllers[outfielderIndex];
	        
            FOutfielderId id = pAiController->GetLastControlledPlayerId();
            AOutfieldCharacter* pLastCharacter = GetOutfielder(id);
            // If this controller is not possessing anything (meaning the Player is in possession)
            // then skip?? (Or should I get the player position???
	        if (pLastCharacter == nullptr)
	        {
                --outfieldersToAssign;
		        AIControllers.Swap(outfielderIndex, outfieldersToAssign);
                continue;
	        }
	        
            const float distanceSqr = (point - pLastCharacter->GetActorLocation()).SquaredLength();
            if (distanceSqr < closestDistanceSqr)
            {
	            closestDistanceSqr = distanceSqr;
                bestOutfielderIndex = outfielderIndex;
            }

            ++outfielderIndex;
        }

        if (bestOutfielderIndex < 0)
        {
	        return;
        }

        // Assign the Formation Point:
        AIControllers[bestOutfielderIndex]->SetFormationPosition(point);
        CurrentFormation->SetPositionAssigned(positionIndex, true);
        --outfieldersToAssign;

        // If this was our last Outfielder to assign, finish
        if (outfieldersToAssign == 0)
	        return;
        
    	// Swap the chosen index to the new end
        AIControllers.Swap(bestOutfielderIndex, outfieldersToAssign);
    }
}

/// Sort the Current Formation's positions so that the highest priority positions are at the front of the Array.
/// @param positions : The Positions
void ATeamManager::SortFormationPositions(TArray<FVector>& positions) const
{
	// Sort the current formation's positions so that the highest priority are first.
    switch (PossessionState)
    {
    	case Neutral:
            break;

    	case InPossession:
    	{
            // Sort the points from Closest to the *Opponent* Goal to furthest
            positions.Sort([this](const FVector left, const FVector right)
            {
                static constexpr float kGoalDistanceScore = 0.95f;
                static constexpr float kBallDistanceScore = 0.25f;

                // Distance to the Opponent Goal
				const FVector goalLocation = RuntimeTeamData.OpponentGoal->GetActorLocation();
	            const FVector toGoalLeft = goalLocation - left; 
	            const FVector toGoalRight = goalLocation - right;

                // Distance to the Ball
                const FVector soccerBallLocation = SoccerBall->GetActorLocation();
                const FVector toBallLeft = soccerBallLocation - left;
                const FVector toBallRight = soccerBallLocation - right;

                const float leftScore =  toGoalLeft.SquaredLength() * kGoalDistanceScore - toBallLeft.SquaredLength() * kBallDistanceScore;
                const float rightScore =  toGoalRight.SquaredLength() * kGoalDistanceScore - toBallRight.SquaredLength() * kBallDistanceScore;

                return leftScore < rightScore;
            });
            break;
    	}

    	case OutOfPossession:
    	{
            // Sort the points from Closest to the *Team* Goal to furthest
            positions.Sort([this](const FVector left, const FVector right)
            {
                static constexpr float kGoalDistanceScore = 0.95f;
                static constexpr float kBallDistanceScore = 0.05f;

                // Distance to Team Goal
                const FVector goalLocation = RuntimeTeamData.TeamGoal->GetActorLocation();
	            const FVector toGoalLeft = goalLocation - left; 
	            const FVector toGoalRight = goalLocation - right;

                // Distance to the Ball
                const FVector soccerBallLocation = SoccerBall->GetActorLocation();
                const FVector toBallLeft = soccerBallLocation - left;
                const FVector toBallRight = soccerBallLocation - right;

                const float leftScore =  toGoalLeft.SquaredLength() * kGoalDistanceScore + toBallLeft.SquaredLength() * kBallDistanceScore;
                const float rightScore =  toGoalRight.SquaredLength() * kGoalDistanceScore + toBallRight.SquaredLength() * kBallDistanceScore;

                return leftScore < rightScore;
            });
            break;
    	}

    	default: break;
    }
}

void ATeamManager::AssignTeamRoles()
{
    // Reset the RoleCounts
    PossessionConfigs[PossessionState].ResetRoleAssignments();

    // Foreach Ai Controller that is controlling a Pawn
    for (size_t i = 0; i < AIControllers.Num(); ++i)
    {
        TObjectPtr<AOutfieldAIController> pAiController = AIControllers[i];

        AOutfieldCharacter* pOutfielder = pAiController->GetControlledOutfielder();
        // If we don't have an outfielder, then we are being controlled by the player, we will handle that case later.
        if (pOutfielder == nullptr)
            continue;

        const ERoleType role = DetermineRoleForOutfielder(pOutfielder);
        pAiController->SetTeamRole(role);
    }

    // Foreach player
    for (size_t i = 0; i < RuntimeTeamData.Players.Num(); ++i)
    {
		TObjectPtr<AOutfieldPlayerController> pPlayerController = RuntimeTeamData.Players[i];
        AOutfieldCharacter* pOutfielder = pPlayerController->GetControlledOutfielder();

        // If we don't have an outfielder, then we are not controlling anyone - skip. 
        if (pOutfielder == nullptr)
            continue;

    	// Get the AI controller that *would* have control when not controlled by the Player.
        AOutfieldAIController* pAiController = pPlayerController->GetOverwrittenAIController();
        const ERoleType role = DetermineRoleForOutfielder(pOutfielder);

        if (pAiController)
            pAiController->SetTeamRole(role);
    }
}

void ATeamManager::UpdatePossessionTime() const
{
    if (PossessionState == ETeamPossessionState::Neutral)
        return;

    check(PossessionTimer);
    const float possessionTime = PossessionTimer->GetTimeElapsed();

    for (const auto pAIController : AIControllers)
    {
	    pAIController->SetPossessionTime(possessionTime);
    }
}

ERoleType ATeamManager::DetermineRoleForOutfielder(const AOutfieldCharacter* pOutfielder)
{
    FPossessionStateConfig& possessionConfig = PossessionConfigs[PossessionState];

    TArray<FScoredRole> scoredRoles;
    FTeamRoleContext context {this, pOutfielder};
    for (auto& [roleType, roleConfig] : possessionConfig.TeamRoles)
    {
        if (roleConfig.RoleQuotaFull())
            continue;

        FScoredRole scoredRole = roleConfig.RoleDecision->GetScore(context);

        if (scoredRole.Score > 0.f)
            scoredRoles.Add(scoredRole);
    }

    // If no other role is available, then return StayInFormation:
    if (scoredRoles.IsEmpty())
		return ERoleType::StayInFormation;

    // Otherwise return the highest scored role.
    float highest = scoredRoles[0].Score;
    size_t bestIndex = 0;

    for (size_t i = 1; i < scoredRoles.Num(); ++i)
    {
	    if (scoredRoles[i].Score > highest)
	    {
		    bestIndex = i;
            highest = scoredRoles[i].Score;
	    }
    }

    const ERoleType result = scoredRoles[bestIndex].RoleType;
    possessionConfig.TeamRoles[result].AssignPlayerToRole();

    return result;
}

/// Get a formation of a specific type.
/// @param key : The type of formation you are querying for.
/// @return 
const UGameFormation* ATeamManager::GetFormation(EFormationType key) const
{
	switch (key)
	{
		case kNeutral: return RuntimeTeamData.Formations.Neutral;
		case kScoredOn: return RuntimeTeamData.Formations.ScoredOn;
		case kAttack: return RuntimeTeamData.Formations.Attack;
		case kDefense: return RuntimeTeamData.Formations.Defense;
	}

    return nullptr;
}

/// If the a Player is in possession of the Ball, this returns the Player. Otherwise it returns the Ball. 
AActor* ATeamManager::GetPlayerInPossessionOrBall() const
{
    if (auto pPlayer = SoccerBall->GetPlayerInPossession())
    {
	    return pPlayer;
    }

    return SoccerBall;
}

/// Returns the player in possession of the ball, or null.
AActor* ATeamManager::GetPlayerInPossession() const
{
	return SoccerBall->GetPlayerInPossession();
}

TObjectPtr<AOutfieldCharacter> ATeamManager::GetOutfielder(const FOutfielderId id) const
{
	return GetOutfielder(id.OutfielderIndex);
}

TObjectPtr<AOutfieldCharacter> ATeamManager::GetOutfielder(const int32 playerIndex) const 
{
    check(playerIndex < Outfielders.Num());
    return Outfielders[playerIndex];
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///     @brief : Get the closest teammate to another player.
///     @param id : ID of the Player who is asking the question.
///     @return : Pointer to the Teammate, or nullptr on fail.
//----------------------------------------------------------------------------------------------------
TObjectPtr<AOutfieldCharacter> ATeamManager::GetClosestTeammate(const FOutfielderId id) const
{
    if (RuntimeTeamData.TeamSize == 1)
        return nullptr;

    if (RuntimeTeamData.TeamSize == 2)
    {
        for (const auto& pTeammate : Outfielders)
        {
            if (pTeammate->GetOutfielderId() != id)
                return pTeammate;
        }
    }

    // Get the Outfielder by that Id.
    TObjectPtr<AOutfieldCharacter> pOutfielder = GetOutfielder(id);
    if (!pOutfielder)
        return nullptr;

    const FOutfielderId closestId = FieldState.GetClosestTeammate(pOutfielder->GetOutfielderId());
    return GetOutfielder(closestId);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Get the closest Teammate to a Location.
///		@param pOutfielder : Outfielder whose team we are querying.
///		@param location : Location to test for.
///		@returns : Ptr to the closest teammate. Will return nullptr if there are no teammates.
//----------------------------------------------------------------------------------------------------
TObjectPtr<AOutfieldCharacter> ATeamManager::GetClosestTeammateToLocation(const AOutfieldCharacter* pOutfielder, const FVector& location) const
{
    check(pOutfielder != nullptr);

    if (RuntimeTeamData.TeamSize == 1)
        return nullptr;

    if (RuntimeTeamData.TeamSize == 2)
    {
        for (const auto& pTeammate : Outfielders)
        {
            if (pTeammate != pOutfielder)
                return pTeammate;
        }
    }

    const FOutfielderId id = pOutfielder->GetOutfielderId();
    const FOutfielderId closestId = FieldState.GetClosestTeammateToLocation(id, location);
    return GetOutfielder(closestId);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///     @brief : Get the closest player on this team to a given location.
///     @param location : Location we are comparing to.
///     @return : Pointer to the closest player.
//----------------------------------------------------------------------------------------------------
TObjectPtr<AOutfieldCharacter> ATeamManager::GetClosestOutfielderToLocation(const FVector location) const
{
    check(!Outfielders.IsEmpty());

    const FOutfielderId id = FieldState.GetClosestOutfielderToLocation(TeamTag, location);
    return GetOutfielder(id);
}

bool ATeamManager::LocationInTeamHalf(const FVector position) const
{
    return FieldState.LocationInTeamHalf(TeamTag, position);
}

bool ATeamManager::OutfielderInTeamHalf(const AOutfieldCharacter* pOutfielder) const 
{
    check(pOutfielder != nullptr);

    // CONSIDER: LocationInTeamHalf isn't that expensive, so we can use that really.
    return FieldState.OutfielderInTeamHalf(pOutfielder->GetOutfielderId());
	//return LocationInTeamHalf(pOutfielder->GetActorLocation());
}

void ATeamManager::GetRespawnPositionAndRotation(const FOutfielderId id, FVector& position, FRotator& rotation) const
{
    check(Outfielders.Num() > id.OutfielderIndex);

    // TODO: Revisit when you get the Restart timing correct.
    const auto& formation = CurrentFormation;
    position = formation->GetLocationOfPoint(id.OutfielderIndex);
	rotation = formation->GetFormationRotation();
}

//----------------------------------------------------------------------------------------------------
//		TODO: Move to the FieldManager.
///		@brief : Sets the Formation rotation based on the TeamGoal, and sets Debug colors.
//----------------------------------------------------------------------------------------------------
void ATeamManager::InitFormations() const
{
    const FRotator teamRotation = RuntimeTeamData.TeamGoal->GetActorRotation();
    auto& formations = RuntimeTeamData.Formations;

    // Set the Rotations:
    formations.Neutral->SetFormationRotation(teamRotation);
    formations.ScoredOn->SetFormationRotation(teamRotation);
    formations.Attack->SetFormationRotation(teamRotation);
    formations.Defense->SetFormationRotation(teamRotation);

    // Debug Colors:
    formations.ScoredOn->SetDebugColors(ActivePositionColor, UnassignedPositionColor);
    formations.Neutral->SetDebugColors(ActivePositionColor, UnassignedPositionColor);
    formations.Attack->SetDebugColors(ActivePositionColor, UnassignedPositionColor);
    formations.Defense->SetDebugColors(ActivePositionColor, UnassignedPositionColor);
}

void ATeamManager::ResetTeam()
{
    ResetAllFormations();
	ResetAllRoles();
    PossessionState = Neutral;

    // Reset the positions of all the players.
    for (size_t i = 0; i < Outfielders.Num(); ++i)
    {
        auto pOutfielder = Outfielders[i];

	    FVector position;
		FRotator rotation;
        GetRespawnPositionAndRotation(pOutfielder->GetOutfielderId(), position, rotation);
        position.Z = pOutfielder->GetActorLocation().Z;

        pOutfielder->GetCharacterMovement()->Velocity = FVector::ZeroVector;
		pOutfielder->SetActorLocationAndRotation(position, rotation);
    }
}

void ATeamManager::ResetAllFormations() const
{
    const auto& formations = RuntimeTeamData.Formations;
    formations.Attack->ClearAllAssignments();
    formations.Defense->ClearAllAssignments();
    formations.Neutral->ClearAllAssignments();
    formations.ScoredOn->ClearAllAssignments();
}

void ATeamManager::ResetAllRoles()
{
	for (auto& [possessionType, possessionConfig] : PossessionConfigs)
	{
		possessionConfig.ResetRoleAssignments();
	}
}

void ATeamManager::OnTeamPossessionChanged(FGameplayTag teamInPossessionTag)
{
    CurrentFormation->ClearAllAssignments();

    // If the tag is empty, that means the possession is neutral.
    // We don't set the formation here, so that it looks more natural as the players go for the ball.
    if (teamInPossessionTag == FGameplayTag::EmptyTag)
    {
        GEngine->AddOnScreenDebugMessage(41, 5.f, FColor::Green, TEXT("Possession Neutral!"));
		PossessionState = Neutral;
    }

    // If we are now in possession, set our current formation to attack.
	else if (teamInPossessionTag == TeamTag)
	{
        GEngine->AddOnScreenDebugMessage(41, 5.f, FColor::Blue, FString::Printf(TEXT("%s gained Possession!"), *TeamTag.ToString()));
        PossessionState = InPossession;
		CurrentFormation = RuntimeTeamData.Formations.Attack;
	}

    // Otherwise, we are not in possession, set our current formation to defense.
    else
    {
        PossessionState = OutOfPossession;
		CurrentFormation = RuntimeTeamData.Formations.Defense;
    }

    // Update the AI understanding of the Possession State.
    for (size_t i = 0; i < AIControllers.Num(); ++i)
    {
	    AIControllers[i]->SetTeamPossessionState(PossessionState);
    }

    UpdateFormationAndRoles();
}
