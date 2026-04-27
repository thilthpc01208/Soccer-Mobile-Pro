// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MatchState.h"
#include "UObject/Interface.h"
#include "MatchEntityInterface.generated.h"

class AProjectSoccerGameMode;

//----------------------------------------------------------------------------------------------------
///		@brief : Describes the different stages of the soccer match.
//----------------------------------------------------------------------------------------------------
UENUM()
enum class EMatchPhase : uint8
{
    Null,       // Invalid phase.
    PreMatch,   // PreMatch operations are underway.
    Kickoff,    // Teams are being reset after scoring a goal.
    Playing,    // The match is underway.
    PostMatch,  // PostMatch operations are underway.
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMatchEntityInterface : public UInterface
{
	GENERATED_BODY()
};

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Classes that implement this interface will be able to receive events about the
/// 			 state of the Match, and can be properly managed by the GameMode.
///     @note : Overridden functions need to be public so that the GameMode can properly initialize
///             Entities when not created during the Initialization step.
//----------------------------------------------------------------------------------------------------
class PROJECTSOCCER_API IMatchEntityInterface
{
	GENERATED_BODY()

public:
	//----------------------------------------------------------------------------------------------------
	//		NOTES:
	//		
	///		@brief : Called when the InitMatchEntities Event is broadcast by the GameMode, or when Entities are created 
	///            by the GameMode. Example: A new Player is added to the Game. The GameMode will
    ///            first call <code>OnMatchInit()</code> to initialize the new PlayerController, then
    ///            call <code>OnMatchPhaseBegin()</code> to notify the PlayerController of the current
    ///            Match Phase.
	///		@param pGameMode : Ptr to the GameMode object.
	//----------------------------------------------------------------------------------------------------
	virtual void OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode) = 0;

	//----------------------------------------------------------------------------------------------------
	//		NOTES:
	//		
	///		@brief : Called when a new MatchPhase begins.
	///		@param phase : Current Phase of the Match.
	//----------------------------------------------------------------------------------------------------
	virtual void OnMatchPhaseBegin(const EMatchPhase phase) = 0;
};