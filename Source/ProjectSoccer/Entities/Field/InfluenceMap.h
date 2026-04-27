// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_OnCircle.h"
#include "GameFramework/Actor.h"
#include "InfluenceMap.generated.h"

class AOutfieldCharacter;

DECLARE_EVENT(AInfluenceMap, FInfluenceUpdated);
DECLARE_EVENT(AInfluenceMap, FGridUpdated);

struct FInfluenceAgentInfo
{
    FVector WorldLocation;
    float Value;
    float Radius;
};

UENUM(BlueprintType)
enum ETargetInfluenceQueryType
{
	Team,
    Opponent,
    NoInfluence
};

USTRUCT(BlueprintType)
struct FInfluencePointResult
{
	GENERATED_BODY()

    FVector Location;
    float Influence;
};

USTRUCT(BlueprintType)
struct FInfluenceArcQueryParams
{
	GENERATED_BODY()

    /** The radius of the Arc.*/
    UPROPERTY(EditAnywhere)
    float Radius = 200.f;

    /** The total arc length in degrees. A value of 360 is a full circle.*/
    UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 360.f))
    float ArcDegrees = 360.f;

	/** Items will be generated on an arc this much apart */
    UPROPERTY(EditAnywhere)
    float SpaceBetween = 50.f;

    /** This many items will be generated on an arc. */
    UPROPERTY(EditAnywhere)
    int32 NumberOfPoints = 8;

    /** How the points on the arc will be generated. */
    UPROPERTY(EditAnywhere)
    EPointOnCircleSpacingMethod SpacingMethod = EPointOnCircleSpacingMethod::BySpaceBetween;
};

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : The InfluenceMap is grid-based.
///        Use <code>CalculateMap()</code> to calculate the influence values of a set number of agent. It
///        is meant to be run on a separate thread.
///        Use <code>CreateGrid()</code> to initialize the grid of the InfluenceMap - it must be called before
///        <code>CalculateMap()</code>.
//----------------------------------------------------------------------------------------------------
class FInfluenceMap
{
    TArray<FBox> GridTiles;		        // Array of grid tiles.
    TArray<float> InfluenceMap;         // Array of influence values.
    FVector MapLocation;                // Location of the Map in World Space.
    FIntPoint Resolution{};		        // The density of tiles within the map's volume, in the X and Y direction.
    FVector2D GridSize{};		        // The size of the grid.
    FVector2D TileSize{};		        // The size of each tile.

    // TODO: Decay and Momentum?
    /*Controls how quickly the influence values will decay toward 0.*/
    //float Decay = 1.f;

    /*If you set the momentum to high (closer to 1.0) then the algorithm
	will bias towards the historical values of the influence, which is
	particularly well suited to storing statistics about previous attacks.
	Use this for things like high-level strategic maps. Conversely, if
	you set the momentum parameter to low (closer to 0.0) then the algorithm
	biases towards the currently calculated influence, so the propagation
	happens quicker and the prediction is more accurate. Use this for
	low-level influence maps for individual positioning for example.*/
    //float Momentum = 0.5f;

public:
    // Initialization:
    void CreateGrid(const FVector& mapLocation, const FIntPoint& resolution, const FVector2D& gridSize);
    void CalculateMap(const TArray<FInfluenceAgentInfo>& agentInfo);

    // Queries:
    bool GetInfluenceAtPosition(const FVector& worldPosition, float& result) const;
    TArray<FInfluencePointResult> GetInfluencePointsOnArc(const FVector& center, const FVector& direction, const FInfluenceArcQueryParams& params) const;
    bool CalcAverageInfluenceInArc(const FVector& center, const FVector& direction, const FInfluenceArcQueryParams& params, float& outResult) const;
    
    // For Debug Draw:
    const TArray<FBox>& GetGridTiles() const { return GridTiles; }
    const TArray<float>& GetInfluenceArray() const { return InfluenceMap; }
    const FVector& GetMapLocation() const { return MapLocation; }

private:
    void AddInfluence(TArray<float>& influenceMap, const int startIndex, const FInfluenceAgentInfo& agent);
    int GetIndexOfTileAtWorldPosition(const FVector& worldPosition) const;
    bool GetGridCoordinatesAtWorldPosition(const FVector& worldPosition, int& xPos, int& yPos) const;
    std::array<int, 4> GetNeighbors(const int index) const;
    int GetIndexOrNull(int xPos, int yPos) const;
    int GetGridIndex(const int x, const int y) const { return y + x * Resolution.Y; };
    int GetGridXPos(const int index) const { return index / Resolution.Y; }
    int GetGridYPos(const int index) const { return index % Resolution.Y; }
};
