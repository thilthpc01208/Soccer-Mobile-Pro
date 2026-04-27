// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/Entities/Field/FieldManager.h"

#include "ProjectSoccer/Entities/Field/GoalTrigger.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"
#include "ProjectSoccer/GameMode/ProjectSoccerGameMode.h"

AFieldManager::AFieldManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    BuildingState = new UFieldState();
    ReadyState = new UFieldState();
}

void AFieldManager::BeginPlay()
{
	Super::BeginPlay();

    AProjectSoccerGameMode::OnMatchInit().AddUObject(this, &AFieldManager::OnMatchInit);
    AProjectSoccerGameMode::OnMatchPhaseBegin().AddUObject(this, &AFieldManager::OnMatchPhaseBegin);
    AProjectSoccerGameMode::AddOnRequestMatchPhaseListener(this, &AFieldManager::OnMatchPhaseRequested);
}

void AFieldManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CheckFieldStateReady();
}

void AFieldManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    AProjectSoccerGameMode::RemoveOnRequestMatchPhaseListener(this);
    AProjectSoccerGameMode::OnMatchInit().RemoveAll(this);
    AProjectSoccerGameMode::OnMatchPhaseBegin().RemoveAll(this);

    // Delete the FieldStates.
    delete BuildingState;
    BuildingState = nullptr;

    delete ReadyState;
    ReadyState = nullptr;
}

void AFieldManager::OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode)
{
    GameMode = pGameMode;
    HomeTag = pGameMode->GetHomeTag();
    AwayTag = pGameMode->GetAwayTag();

    // Get the two Team Managers references.
    HomeTeamManager = pGameMode->GetTeamManager(HomeTag);
    AwayTeamManager = pGameMode->GetTeamManager(AwayTag);

    // Set the Tags for the Goal Triggers
    HomeGoal->SetScoringTeamTag(AwayTag);
    AwayGoal->SetScoringTeamTag(HomeTag);

    FieldStateTask = {};

    // Get the Outfielders from the Team Managers.
    Outfielders = HomeTeamManager->GetObjectPtrOutfielders();
    Outfielders.Append(AwayTeamManager->GetObjectPtrOutfielders());

    // Initialize Base Data for the FieldState.
    BuildingState->OutfielderStates.Reserve(Outfielders.Num());
    ReadyState->OutfielderStates.Reserve(Outfielders.Num());

    // Set the Outfielders
    size_t i = 0;
    for (auto pOutfielder : Outfielders)
    {
        FOutfielderStateInfo stateInfo;
        stateInfo.Id = pOutfielder->GetOutfielderId();
        stateInfo.Location = pOutfielder->GetActorLocation();

        BuildingState->OutfielderIdToIndex.Add(stateInfo.Id, i);
        BuildingState->OutfielderStates.Add(stateInfo);

        ReadyState->OutfielderIdToIndex.Add(stateInfo.Id, i);
        ReadyState->OutfielderStates.Add(stateInfo);

        ++i;
    }

    // Set the Influence Grid Data:
    const FVector mapLocation = GetActorLocation();
    ReadyState->InfluenceMap.CreateGrid(mapLocation, InfluenceMapResolution, FieldSize);
    BuildingState->InfluenceMap.CreateGrid(mapLocation, InfluenceMapResolution, FieldSize);

    // Set the Field Dimensions, which will not change.
    constexpr float kFieldHeight = 500.f; // Tune this...
    constexpr float kHalfFieldHeight = kFieldHeight * 0.5f;
    const FVector extent = FVector(FieldSize.X * 0.5f, FieldSize.Y * 0.5f, kHalfFieldHeight);
    const FVector fieldMin = mapLocation - extent;
    const FVector fieldMax = mapLocation + extent;
    const FBox fieldDimensions{ fieldMin, fieldMax };
    ReadyState->FieldDimensions = fieldDimensions;
    BuildingState->FieldDimensions = fieldDimensions;

    // Set the Goal State Data, which will not change.
    ReadyState->HomeGoal = { HomeGoal->GetActorLocation(), HomeGoal->GetActorForwardVector(), HomeTag };
    ReadyState->AwayGoal = { AwayGoal->GetActorLocation(), AwayGoal->GetActorForwardVector(), AwayTag };
    BuildingState->HomeGoal = { HomeGoal->GetActorLocation(), HomeGoal->GetActorForwardVector(), HomeTag };
    BuildingState->AwayGoal = { AwayGoal->GetActorLocation(), AwayGoal->GetActorForwardVector(), AwayTag };

    OnFieldDimensionsUpdatedEvent.Broadcast(GetFieldSize(), ReadyState->InfluenceMap.GetGridTiles());
}

void AFieldManager::OnMatchPhaseBegin(const EMatchPhase phase)
{
    switch (phase)
    {
    	case EMatchPhase::Kickoff:
	    case EMatchPhase::Playing:
	    {
	        break;
	    }

	    default:
	    {
            // Null out the FieldStateTask when we are not in the Playing phase.
            FieldStateTask = {};
	    }
    }
}

void AFieldManager::OnMatchPhaseRequested(const TObjectPtr<AProjectSoccerGameMode>& pGameMode, const EMatchPhase phase)
{
    switch (phase)
    {
    	case EMatchPhase::Playing:
    	{
            // [Maybe Later] Add this to start ticking?
            // - We don't need it to tick until we are in the Playing phase.
            //SetActorTickEnabled(true);

			// Before we can play, we need to cache the first FieldState.
            // We don't notify the GameMode until the FieldState is ready.
            BeginBuildFieldState();
    		break;
    	}

    	default:
    	{
    		pGameMode->NotifyReadyForNextPlayState();
    	}
    }
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Create a Snapshot of the Field and Outfielders, then launch the Task to build the FieldState.
//----------------------------------------------------------------------------------------------------
void AFieldManager::BeginBuildFieldState()
{
    using namespace UE::Tasks;

    // Create the Snapshot.
    FFieldSnapshot fieldSnapshot;

    // Create and add each Outfielder to the field Snapshot.
    for (auto pOutfielder : Outfielders)
    {
        FOutfielderSnapshot outfielderSnapshot;
        outfielderSnapshot.Id = pOutfielder->GetOutfielderId();
        outfielderSnapshot.WorldLocation = pOutfielder->GetActorLocation();
        outfielderSnapshot.PassTargetLocation = pOutfielder->GetPassTargetLocation();
        outfielderSnapshot.InfluenceValue = pOutfielder->GetInfluenceValue();
        outfielderSnapshot.InfluenceRadius = pOutfielder->GetInfluenceRadius();

        fieldSnapshot.Outfielders.Add(outfielderSnapshot);
    }

    // Launch a Task to calculate the FieldState.
    FieldStateTask = Launch(TEXT("Building Field State"), [this, fieldSnapshot]()
    {
    	BuildFieldState(BuildingState, fieldSnapshot);
    });
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : If the FieldStateTask is completed, swap the BuildingState with the ReadyState, and
///             give the new ReadyState to the Team Managers.
//----------------------------------------------------------------------------------------------------
void AFieldManager::CheckFieldStateReady()
{
    if (FieldStateTask.IsCompleted())
    {
        // Swap the BuildFieldState with the ReadyFieldState.
        auto* pTemp = BuildingState;
        BuildingState = ReadyState;
        ReadyState = pTemp;

        check(ReadyState != nullptr);
        
        // Set the field state of the teams directly - we already have a reference to them.
        HomeTeamManager->SetFieldState(*ReadyState);
        AwayTeamManager->SetFieldState(*ReadyState);
        
        // If we are waiting on the first FieldState,
        // Notify the GameMode that this is to begin Playing.
        if (GameMode->GetRequestedMatchPhase() == EMatchPhase::Playing)
        {
            GameMode->NotifyReadyForNextPlayState();
        }

        // Broadcast the new state:
        OnFieldStateUpdated().Broadcast(*ReadyState);

        // Trigger the next build.
        BeginBuildFieldState();
    }
}

#if WITH_EDITOR
void AFieldManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName propertyName = PropertyChangedEvent.GetPropertyName();

    // Field Dimensions updated
    if (propertyName == GET_MEMBER_NAME_CHECKED(AFieldManager, FieldSize) ||
        propertyName == GET_MEMBER_NAME_CHECKED(AFieldManager, InfluenceMapResolution))
    {
        check(ReadyState != nullptr);

        // Update the Influence Grid Data:
        const FVector mapLocation = GetActorLocation();
        ReadyState->InfluenceMap.CreateGrid(mapLocation, InfluenceMapResolution, FieldSize);
        BuildingState->InfluenceMap.CreateGrid(mapLocation, InfluenceMapResolution, FieldSize);

        // Refresh the Debug View:
        OnFieldDimensionsUpdatedEvent.Broadcast(GetFieldSize(), ReadyState->InfluenceMap.GetGridTiles());
    }

    // Debug View updated
    //else if (propertyName == GET_MEMBER_NAME_CHECKED(AFieldManager, CurrentDebugView))
    //{
    //    check(ReadyState != nullptr);

    //    // Refresh the Debug View
    //    OnDebugViewNeedsRefresh.Broadcast(GetFieldSize(), ReadyState->InfluenceMap.GetGridTiles());
    //}
}
#endif

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Run the FieldState calculation based on the current Snapshot of the Field.
///		@param pBuildingState : Field state that we are updating.
///		@param snapshot : Current snapshot of the field.
//----------------------------------------------------------------------------------------------------
void AFieldManager::BuildFieldState(UFieldState* pBuildingState, const FFieldSnapshot& snapshot)
{
    check(pBuildingState != nullptr);

    // NOTE: If you wanted the ability to add outfielders at runtime, then you need to handle this case.
    // Ensure that the OutfielderStates are the same size as the Outfielders in the Snapshot,
    // the states should be set up in Match Init.
    check(snapshot.Outfielders.Num() == pBuildingState->OutfielderStates.Num());

    TArray<FInfluenceAgentInfo> agents{};
    agents.Reserve(snapshot.Outfielders.Num());

    // For each Outfielder in the Snapshot, update their OutfielderStateInfo.
    //for (const auto& outfielderSnapshot : snapshot.Outfielders)
    for (size_t i = 0; i < snapshot.Outfielders.Num(); ++i)
    {
        const FOutfielderSnapshot& outfielderSnapshot = snapshot.Outfielders[i];

        // Add the Agent for the Influence Map calculation.
        agents.Add(FInfluenceAgentInfo{ outfielderSnapshot.WorldLocation, outfielderSnapshot.InfluenceValue, outfielderSnapshot.InfluenceRadius });

        // Get the StateInfo for the current Outfielder.
        const size_t stateIndex = pBuildingState->OutfielderIdToIndex[outfielderSnapshot.Id];
        FOutfielderStateInfo& outfielderState = pBuildingState->OutfielderStates[stateIndex];

        // Location
        outfielderState.Location = outfielderSnapshot.WorldLocation;
        outfielderState.PassTargetLocation = outfielderSnapshot.PassTargetLocation;

        // TeamHalf
        // Determine whether the Outfielder is in their team's half of the field.
        SetInTeamHalf(pBuildingState, snapshot, outfielderState);

        // Closest Outfielders
        for (size_t j = 0; j < snapshot.Outfielders.Num(); ++j)
        {
            // If this is the same Outfielder, skip it.
            const auto& otherOutfielder = snapshot.Outfielders[j];
            if (outfielderSnapshot.Id == otherOutfielder.Id)
            {
                continue;
            }

            // CONSIDER: This can be distance squared if we don't care about the actual distance.
            // TODO: Does the Jumping mechanic need to be considered?
            // Calculate the distance between the two Outfielders.
            const FVector toOutfielder = otherOutfielder.WorldLocation - outfielderSnapshot.WorldLocation;
            const float distance = toOutfielder.Length();

            // Add the Outfielder to the ClosestOutfielders list.
            outfielderState.ClosestOutfielders.Add({ otherOutfielder.Id, toOutfielder.GetSafeNormal(), distance });
        }

        // Sort the array so that the closest Outfielders are first.
        outfielderState.ClosestOutfielders.Sort([](const FOutfielderStateInfo::OutfielderDistanceInfo& a, const FOutfielderStateInfo::OutfielderDistanceInfo& b)
            {
                return a.Distance < b.Distance;
            });
    }

    // Run the Influence Map Calculation.
    pBuildingState->InfluenceMap.CalculateMap(agents);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Set the InTeamHalf boolean on the OutfielderStateInfo.
///     @param pBuildingState : Field state that we are updating.
///		@param snapshot : Snapshot of the field.
///		@param stateInfo : Current State of the Outfielder. The Location must be set prior to this call.
//----------------------------------------------------------------------------------------------------
void AFieldManager::SetInTeamHalf(const UFieldState* pBuildingState, const FFieldSnapshot& snapshot, FOutfielderStateInfo& stateInfo)
{
    // Kinda Hacky for now.
    // The Stadium's origin is at FVector::Zero, so the normalized forward
	// of the TeamManager (Context) and the Outfielder location (Some point) can act as the direction vectors.
    const FVector teamForward = stateInfo.Id.TeamTag == pBuildingState->HomeGoal.OwningTeamTag ? pBuildingState->HomeGoal.FacingDirection : pBuildingState->AwayGoal.FacingDirection;
	const float dotProduct = FVector::DotProduct(teamForward, stateInfo.Location);            

    // The Outfielder is in their team's half if the dot product is negative.
    stateInfo.InTeamHalf = dotProduct < 0.f;
}

////----------------------------------------------------------------------------------------------------
////		NOTES:
////		TODO: I need to update the ReadyState's OutfielderStates array.
////        The issue here is that the ReadyState that the TeamManagers are using, is now not
////        valid for the new Outfielders that have been added.
////        I would need to have some sort of way of notifying the TeamManagers that the FieldState is dirty
////        or the Outfielders aren't into the game until their StateInfo is valid?
////
/////		@brief : Add Outfielders to the FieldManager that will be part of the FieldState calculations.
/////		@param teamTag : 
/////		@param outfielders : Array of Outfielders to add to the FieldManager.
////----------------------------------------------------------------------------------------------------
//void AFieldManager::RegisterOutfielders(const FGameplayTag teamTag, const TArray<TObjectPtr<AOutfieldCharacter>>& outfielders)
//{
//	Outfielders.Append(outfielders);
//
//    // Update the ReadyState's OutfielderStates array.
//    // The issue here is that the ReadyState that the TeamManagers are using, is now not
//    // valid for the new Outfielders that have been added.
//    // I would need to have some sort of way of notifying the TeamManagers that the FieldState is dirty
//    // or the Outfielders aren't into the game until their StateInfo is valid?
//    ReadyState->OutfielderStates.Reserve(Outfielders.Num());
//
//    for (const auto& outfielder : outfielders)
//    {
//        FOutfielderId id = outfielder->GetOutfielderId();
//
//		if (!ReadyState->HasOutfielder(id))
//		{
//			ReadyState->AddOutfielder(id);
//		}
//    }
//    
//}

//void AFieldManager::BuildFieldStateAsync()
//{
//    // TODO: Have a param that is a GameState to pass into the FieldStateBuilder
//
//    // Begin the Thread.
//    AsyncTask(ENamedThreads::Type::AnyBackgroundThreadNormalTask, [&]()
//    {
//		// TODO: Should we cache this task on the AFieldManager object?
//        // That way we can grab the completed FieldState from the AFieldManager object.
//
//        auto* pTask = new FAsyncTask<FAsyncFieldStateBuilder>(); // TODO: Pass in the GameState into the constructor.
//        pTask->StartBackgroundTask(GThreadPool, EQueuedWorkPriority::Normal, EQueuedWorkFlags::None, -1, TEXT("BuildingFieldState"));
//        pTask->EnsureCompletion(); 
//        
//        delete pTask;
//    });
//}

////----------------------------------------------------------------------------------------------------
////		NOTES:
////		
/////		@brief : Required function for the Async Task to do work.
////----------------------------------------------------------------------------------------------------
//void FAsyncFieldStateBuilder::DoWork()
//{
//	// TODO: 
//}
