// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/Entities/Field/FieldState.h"

#include "FieldManager.h"

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      O(N) where is the number of Outfielders - 1, but we are just comparing pointers.
//		
///		@brief : Get the closest teammate to the Outfielder.
///		@param outfielderId : ID of the Outfielder to find the closest teammate to.
///		@returns : If there are no teammates, it will return the passed in outfielderId.
//----------------------------------------------------------------------------------------------------
FOutfielderId UFieldState::GetClosestTeammate(const FOutfielderId outfielderId) const
{
    auto& outfielderState = GetOutfielderState(outfielderId);

    // Return the first closest teammate.
    for (auto& distanceInfo : outfielderState.ClosestOutfielders)
    {
        if (distanceInfo.Id.TeamTag == outfielderId.TeamTag)
            return distanceInfo.Id;
    }

    // Return self if there are no teammates.
    return outfielderId;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      0(N) for the total number of outfielders, but the calculation is simple.
//		
///		@brief : Returns the closest teammate to a Location, skipping the passed in OutfielderId.
///		@param outfielderId : The Outfielder that is looking for the closest teammate.
///		@param location : Location to find the closest teammate to.
///		@returns : OutfielderId of the closest teammate. If there are no teammates, it will return
///             the passed in outfielderId.
//----------------------------------------------------------------------------------------------------
FOutfielderId UFieldState::GetClosestTeammateToLocation(const FOutfielderId outfielderId, const FVector& location) const
{
    check(OutfielderIdToIndex.Contains(outfielderId));

    float distanceSquared = TNumericLimits<float>::Max();
    FOutfielderId resultId = outfielderId;

    for (auto& outfielderState : OutfielderStates)
    {
        // If outfielderState represents someone on the opposite team or is the same outfielder, skip.
        if (outfielderState.Id.TeamTag != outfielderId.TeamTag || outfielderState.Id.OutfielderIndex == outfielderId.OutfielderIndex)
            continue;

        float newDistanceSquared = FVector::DistSquared(outfielderState.Location, location);
        if (newDistanceSquared < distanceSquared)
        {
            distanceSquared = newDistanceSquared;
            resultId = outfielderState.Id;
        }
    }

    return resultId;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Get the Closest Teammate in a Direction. Filters out any Teammates that are not within
///             the dotThreshold.
///		@param outfielderId : ID of the Outfielder to find the closest teammate to.
///		@param direction : Direction that we are querying.
///		@param dotThreshold : Value between -1 and 1. 1 is the same direction, 0 is perpendicular, -1 is opposite. Default is 0.
///		@returns : If there are no valid teammates in the direction, it will return an invalid OutfielderId.
//----------------------------------------------------------------------------------------------------
FOutfielderId UFieldState::GetClosestTeammateInDirection(const FOutfielderId outfielderId, const FVector& direction, const float dotThreshold) const
{
    check(OutfielderIdToIndex.Contains(outfielderId));

    float lowestDistanceSquared = TNumericLimits<float>::Max();
    FOutfielderId resultId{};

    for (const auto& outfielderState : OutfielderStates)
    {
        // If outfielderState represents someone on the opposite team or is the same outfielder, skip.
        if (outfielderState.Id.TeamTag != outfielderId.TeamTag || outfielderState.Id.OutfielderIndex == outfielderId.OutfielderIndex)
            continue;

        const FVector toTeammate = outfielderState.Location - GetOutfielderState(outfielderId).Location;
        const float dot = FVector::DotProduct(toTeammate.GetSafeNormal(), direction);
        const float distSquared = FVector::DistSquared(outfielderState.Location, GetOutfielderState(outfielderId).Location);

        if (dot > dotThreshold && distSquared < lowestDistanceSquared)
        {
            lowestDistanceSquared = distSquared;
            resultId = outfielderState.Id;
        }
    }

    return resultId;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Get the closest Pass Location for each Teammate in a Direction. Filters out any Teammates
///              that are not within the dotThreshold.
///		@param outfielderId : ID of the Outfielder to find the closest teammate to.
///		@param direction : Direction that we are querying.
///		@param dotThreshold : Value between -1 and 1. 1 is the same direction, 0 is perpendicular, -1 is opposite. Default is 0.
///		@returns : If there are no valid teammates in the direction, it will return an invalid OutfielderId.
//----------------------------------------------------------------------------------------------------
FOutfielderId UFieldState::GetClosestTeammatePassInDirection(const FOutfielderId outfielderId, const FVector& direction, const float dotThreshold) const
{
    check(OutfielderIdToIndex.Contains(outfielderId));

    float lowestDistanceSquared = TNumericLimits<float>::Max();
    FOutfielderId resultId{};

    for (const auto& outfielderState : OutfielderStates)
    {
        // If outfielderState represents someone on the opposite team or is the same outfielder, skip.
        if (outfielderState.Id.TeamTag != outfielderId.TeamTag || outfielderState.Id.OutfielderIndex == outfielderId.OutfielderIndex)
            continue;

        const FVector& passTargetLocation = GetOutfielderState(outfielderId).PassTargetLocation;
        const FVector toTeammatePassLocation = outfielderState.Location - passTargetLocation;
        const float dot = FVector::DotProduct(toTeammatePassLocation.GetSafeNormal(), direction);
        const float distSquared = FVector::DistSquared(outfielderState.Location, passTargetLocation);

        if (dot > dotThreshold && distSquared < lowestDistanceSquared)
        {
            lowestDistanceSquared = distSquared;
            resultId = outfielderState.Id;
        }
    }

    return resultId;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Return the Closest Outfielder on a specific Team to a Location.
///		@param teamTag : What team should be queried.
///		@param location : World Location to compare to.
///		@returns : OutfielderId of the closest Outfielder on the team. This is Invalid only if
///         there are no Outfielders on the team.
//----------------------------------------------------------------------------------------------------
FOutfielderId UFieldState::GetClosestOutfielderToLocation(const FGameplayTag teamTag, const FVector& location) const
{
    FOutfielderId resultId = {};
	float distanceSquared = TNumericLimits<float>::Max();

    for (auto& outfielderState : OutfielderStates)
    {
        // If outfielderState represents someone on the opposite team or is the same outfielder, skip.
        if (outfielderState.Id.TeamTag != teamTag)
            continue;

        float newDistanceSquared = FVector::DistSquared(outfielderState.Location, location);
        if (newDistanceSquared < distanceSquared)
        {
            distanceSquared = newDistanceSquared;
            resultId = outfielderState.Id;
        }
    }

    return resultId;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      O(1) as it is just a lookup.
//		
///		@brief : Return whether an Outfielder is in their team's half of the field.
///		@param outfielderId : ID of the Outfielder to check.
///		@returns : True if the Outfielder is in their team's half of the field.
//----------------------------------------------------------------------------------------------------
bool UFieldState::OutfielderInTeamHalf(const FOutfielderId outfielderId) const
{
    auto& outfielderState = GetOutfielderState(outfielderId);
    return outfielderState.InTeamHalf;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Return whether a Location is in a team's half of the field.
///     @note : If you are asking about an Outfielder's location, use OutfielderInTeamHalf.
///		@param teamTag : 
///		@param location : 
///		@returns : 
//----------------------------------------------------------------------------------------------------
bool UFieldState::LocationInTeamHalf(const FGameplayTag teamTag, const FVector& location) const
{
    const auto& goal = teamTag == HomeGoal.OwningTeamTag ? HomeGoal : AwayGoal;
    const FVector teamForward = goal.FacingDirection;
	const float dotProduct = FVector::DotProduct(teamForward, location);            

    return dotProduct < 0.f;
}

//----------------------------------------------------------------------------------------------------
///		@brief : Get a ref to an OutfielderStateInfo based on an OutfielderId.
//----------------------------------------------------------------------------------------------------
const FOutfielderStateInfo& UFieldState::GetOutfielderState(const FOutfielderId outfielderId) const
{
    check(OutfielderIdToIndex.Contains(outfielderId));
    const size_t outfielderIndex = OutfielderIdToIndex[outfielderId];

    check(outfielderIndex < OutfielderStates.Num());
    return OutfielderStates[outfielderIndex];
}