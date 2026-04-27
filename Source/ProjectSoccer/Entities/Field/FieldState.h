// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InfluenceMap.h"
#include "ProjectSoccer/Entities/Outfielder/OutfielderId.h"
//#include "FieldState.generated.h"

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Relational positional data about an Outfielder.
//----------------------------------------------------------------------------------------------------
struct FOutfielderStateInfo
{
    struct OutfielderDistanceInfo
    {
        FOutfielderId Id;
        FVector Direction;
        float Distance;
    };

    TArray<OutfielderDistanceInfo> ClosestOutfielders; // Sorted Closest to Furthest.
    FOutfielderId Id;
    FVector Location;
    FVector PassTargetLocation; // The Location that should be passed to for this Outfielder.
    // TODO: 

	// TODO: Have FieldZones instead of boolean, to allow for more complex decision-making.
    bool InTeamHalf = false;

    // Distance to Goal
    // Distance to Ball
};

struct FGoalStateInfo
{
    FVector Location;
    FVector FacingDirection;
    FGameplayTag OwningTeamTag;
};

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : The Field State object is calculated data based on a snapshot of the current game state.
//----------------------------------------------------------------------------------------------------
class PROJECTSOCCER_API UFieldState
{
    friend class AFieldManager;

    FInfluenceMap InfluenceMap;
    TMap<FOutfielderId, size_t> OutfielderIdToIndex{};
    TArray<FOutfielderStateInfo> OutfielderStates{};
    FGoalStateInfo HomeGoal;
    FGoalStateInfo AwayGoal;
    FBox FieldDimensions;

public:
	UFieldState() = default;
    UFieldState(const UFieldState& right) = default;
    UFieldState(UFieldState&& right) noexcept = default;
    UFieldState& operator=(const UFieldState& right) = default;
    UFieldState& operator=(UFieldState&& right) noexcept = default;

    // Outfielder Distance Queries:
    FOutfielderId GetClosestTeammate(const FOutfielderId outfielderId) const;
    FOutfielderId GetClosestTeammateToLocation(const FOutfielderId outfielderId, const FVector& location) const;
    FOutfielderId GetClosestTeammateInDirection(const FOutfielderId outfielderId, const FVector& direction, const float dotThreshold = 0.f) const;
    FOutfielderId GetClosestTeammatePassInDirection(const FOutfielderId outfielderId, const FVector& direction, const float dotThreshold = 0.f) const;
    FOutfielderId GetClosestOutfielderToLocation(const FGameplayTag teamTag, const FVector& location) const;

    // TODO: Zone Queries
    // Whether an Outfielder is in a certain Zone.
	// Whether a Location is in a certain Zone.
    bool OutfielderInTeamHalf(const FOutfielderId outfielderId) const;
    bool LocationInTeamHalf(const FGameplayTag teamTag, const FVector& location) const;

    const FBox& GetFieldDimensions() const { return FieldDimensions; }

    /** Get the InfluenceMap for Influence-based Queries.*/
    const FInfluenceMap& GetInfluenceMap() const { return InfluenceMap; }

private:
    const FOutfielderStateInfo& GetOutfielderState(const FOutfielderId outfielderId) const;
};
