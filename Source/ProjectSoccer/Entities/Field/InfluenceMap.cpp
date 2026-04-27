// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/Entities/Field/InfluenceMap.h"
#include <array>

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Create the Grid of 2D Tiles that make up the Influence Map. Tiles are created on
///             the XY axis.
///		@param mapLocation : Center Location of the TileMap in World Space.
///		@param resolution : The density of tiles within the map's volume, in the X and Y direction
///		@param gridSize : Size of the Grid in the X and Y direction.
//----------------------------------------------------------------------------------------------------
void FInfluenceMap::CreateGrid(const FVector& mapLocation, const FIntPoint& resolution, const FVector2D& gridSize)
{
    MapLocation = mapLocation;
    Resolution = resolution;
    GridSize = gridSize;

	// Calculate the TileSize.
    TileSize.X = GridSize.X / Resolution.X;
    TileSize.Y = GridSize.Y / Resolution.Y;

    const size_t tileCount = Resolution.X * Resolution.Y;
    InfluenceMap.SetNumZeroed(tileCount);
    GridTiles.SetNumZeroed(tileCount);

    const FVector halfGridSize = FVector(GridSize.X, GridSize.Y, 1.f) * 0.5f;
    FVector start = MapLocation - halfGridSize;

	const FVector tileSize(TileSize.X, TileSize.Y, 1.f);
    const float yStart = start.Y;

    for (size_t i = 0; i < Resolution.X; ++i)
    {
	    start.Y = yStart;

        for (int j = 0; j < Resolution.Y; ++j)
        {
            const int index = GetGridIndex(i, j);
            const FVector min = start;
            const FVector max = start + tileSize;
            GridTiles[index] = FBox(min, max);
            
            //Grid[i].Position = tileOffset;
            //InfluenceGrid[i].Influence = 0.f;
            start.Y += TileSize.Y;
        }

        start.X += TileSize.X;
    }
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Calculate influence values across the grid based on the influence agents.
///		@param agentInfo : Array of agents that have influence on the map.
//----------------------------------------------------------------------------------------------------
void FInfluenceMap::CalculateMap(const TArray<FInfluenceAgentInfo>& agentInfo)
{
	TArray<float> map;
    map.SetNumZeroed(InfluenceMap.Num());

    for (auto& agent : agentInfo)
    {
        const int startIndex = GetIndexOfTileAtWorldPosition(agent.WorldLocation);
        if (startIndex == -1)
            continue;

    	AddInfluence(map, startIndex, agent);
    }

    Swap(InfluenceMap, map);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Get the influence value at a specific world position.
///		@param worldPosition : Position we are querying.
///		@param result : Resulting influence.
///		@returns : False if the position is out of bounds for this InfluenceMap.
//----------------------------------------------------------------------------------------------------
bool FInfluenceMap::GetInfluenceAtPosition(const FVector& worldPosition, float& result) const
{
    const int index = GetIndexOfTileAtWorldPosition(worldPosition);

    if (index < 0)
    {
    	result = 0.f;
		return false;
    }
    
    result = InfluenceMap[index];
    return true;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Get Influence Data for a number of points on the edge of an arc.
///		@param center : Center of the Arc.
///		@param direction : Direction of the Arc.
///		@param params : Parameters to construct the arc and number of points.
///		@returns : Array of valid points and their associated influence value.
//----------------------------------------------------------------------------------------------------
TArray<FInfluencePointResult> FInfluenceMap::GetInfluencePointsOnArc(const FVector& center, const FVector& direction,
	const FInfluenceArcQueryParams& params) const
{
    TArray<FInfluencePointResult> result;

    float influence = 0.f;

    // If we don't have a radius, the arcDegrees == 0.f, or the spacing method and associated value don't make sense,
    // then just return the result of the center point.
    if (params.Radius <= 0.f
        || (params.ArcDegrees <= 0.f)
        || ((params.SpacingMethod == EPointOnCircleSpacingMethod::BySpaceBetween) && (params.SpaceBetween <= 0.f)) 
        || ((params.SpacingMethod == EPointOnCircleSpacingMethod::ByNumberOfPoints) && (params.NumberOfPoints <= 0)))
    {
        if (GetInfluenceAtPosition(center, influence))
        {
	        result.Add({center, influence});
        }

        return result;
    }

    const float circumferenceLength = 2.f * PI * params.Radius;
	const float arcAnglePercentage = params.ArcDegrees / 360.f;
	const float arcLength = circumferenceLength * arcAnglePercentage;

    // Get the number of points we need to generate points for:
	int32 stepCount = 1;
    int32 numStepsBetween = 1;
    switch (params.SpacingMethod)
    {
    	case EPointOnCircleSpacingMethod::ByNumberOfPoints:
    	{
    		stepCount = params.NumberOfPoints;
			numStepsBetween = stepCount;
            break;
    	}

    	case EPointOnCircleSpacingMethod::BySpaceBetween:
    	{
            stepCount = FMath::CeilToInt(arcLength / params.SpaceBetween);
            numStepsBetween = stepCount - 1;
            break;
    	}
    }

    const float angleStep = arcLength / numStepsBetween;
    FVector startDirection = direction;

    // Start on the left half of the arc.
    startDirection = startDirection.RotateAngleAxis(-arcLength / 2.f, FVector::UpVector) * params.Radius;

    // Add each valid point:
    for (int32 step = 0; step < stepCount; ++step)
    {
        const FVector location = center + startDirection.RotateAngleAxis(angleStep * step, FVector::UpVector);

        if (GetInfluenceAtPosition(location, influence))
        {
	        result.Add({location, influence});
        }
    }

    return result;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Calculates all the points on an arc's edge, then takes the average of the influence sampled points.
///		@param center : Center of the Arc.
///		@param direction : Direction of the Arc.
///		@param params : Parameters to construct the Arc.
///     @param outResult : The average influence. This will be invalid if there are no valid points in the arc.
///		@returns : Returns false in the event that no points were valid.
//----------------------------------------------------------------------------------------------------
bool FInfluenceMap::CalcAverageInfluenceInArc(const FVector& center, const FVector& direction,
	const FInfluenceArcQueryParams& params, float& outResult) const
{
    TArray<FInfluencePointResult> points = GetInfluencePointsOnArc(center, direction, params);

    outResult = 0.f;

    if (points.IsEmpty())
        return false;

    for (const auto point : points)
    {
		outResult += point.Influence;   
    }

    outResult /= points.Num();
    return true;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Propagate the influence of an Agent across the Influence Map.
///		@param influenceMap : Map we are updating.
///		@param startIndex : Starting Grid index of the Agent.
///		@param agent : Agent information.
//----------------------------------------------------------------------------------------------------
void FInfluenceMap::AddInfluence(TArray<float>& influenceMap, const int startIndex, const FInfluenceAgentInfo& agent)
{
    static constexpr float kMinimumThreshold = 0.001f;
    if (FMath::Abs(agent.Value) < kMinimumThreshold)
        return;

    TQueue<int> openSet;
    TSet<int> closedSet;
    const float radiusSqr = agent.Radius * agent.Radius;

    openSet.Enqueue(startIndex);
    closedSet.Add(startIndex);

    while (!openSet.IsEmpty())
    {
        int next;
        // This also pops the value.
    	openSet.Dequeue(next);

        // Decrement the influence:
        // TODO: This could be a function passed in.
        //const float neighborInfluence = distanceFalloff * Decay;
        //influence = FMath::Lerp(InfluenceMap[next], influence * 0.5f, Momentum);

        const FVector tileLocation = GridTiles[next].GetCenter();
        const float distanceSqr = (agent.WorldLocation - tileLocation).SquaredLength();
        const float distanceFalloff = FMath::Clamp(1.f - (distanceSqr / radiusSqr), 0.f, 1.f);

        const float falloffInfluence = agent.Value * distanceFalloff;

        if (FMath::Abs(falloffInfluence) < kMinimumThreshold)
            continue;

        // Add the influence.
        influenceMap[next] += falloffInfluence;

        const auto neighbors = GetNeighbors(next);
	    for (const int neighborIndex : neighbors)
	    {
	        // If the index is invalid or already checked, continue.
	        if (neighborIndex == -1 || closedSet.Contains(neighborIndex))
	            continue;

	        closedSet.Add(neighborIndex);
            openSet.Enqueue(neighborIndex);
	    }
    }
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Get the Grid Index of a Tile based on a WorldPosition.
///		@param worldPosition : Position we are querying.
///		@returns : Returns -1 in the event that the position is out of bounds for this InfluenceMap.
///            Otherwise, returns the Grid Index.
//----------------------------------------------------------------------------------------------------
int FInfluenceMap::GetIndexOfTileAtWorldPosition(const FVector& worldPosition) const
{
    int xPos, yPos = -1;
	if (GetGridCoordinatesAtWorldPosition(worldPosition, xPos, yPos))
	{
		return GetGridIndex(xPos, yPos);
	}

    return -1;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Get the X and Y grid coordinates based on a World Position.
///		@param worldPosition : Position we are querying.
///		@param xPos : Out Grid X Position.  
///		@param yPos : Out Grid Y Position.
///		@returns : False if the World Position is out of bounds. In this case, the xPos and yPos
///			will be -1.
//----------------------------------------------------------------------------------------------------
bool FInfluenceMap::GetGridCoordinatesAtWorldPosition(const FVector& worldPosition, int& xPos, int& yPos) const
{
    const FVector localHit = worldPosition - MapLocation;

    xPos = static_cast<int>((localHit.X + GridSize.X * 0.5f) / TileSize.X);
    if (xPos < 0 || xPos >= Resolution.X)
    {
	    xPos = -1;
	    yPos = -1;
        return false;
    }

    yPos = static_cast<int>((localHit.Y + GridSize.Y * 0.5f) / TileSize.Y);
    if (yPos < 0 || yPos >= Resolution.Y)
    {
	    xPos = -1;
	    yPos = -1;
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Get the 4 neighboring tile Indices of a specific tile. If any neighbor is invalid, it will
///             be set to -1.
///		@param index : Index of the tile we are getting neighbors for.
///		@returns : Array of 4 neighboring tile indices.
//----------------------------------------------------------------------------------------------------
std::array<int, 4> FInfluenceMap::GetNeighbors(const int index) const
{
    std::array<int, 4> neighbors {-1, -1, -1, -1};

    const int currentX = GetGridXPos(index);
    const int currentY = GetGridYPos(index);

    neighbors[0] = GetIndexOrNull(currentX + 1, currentY); // UP
    neighbors[1] = GetIndexOrNull(currentX - 1, currentY); // DOWN
    neighbors[2] = GetIndexOrNull(currentX, currentY - 1); // LEFT
    neighbors[3] = GetIndexOrNull(currentX, currentY + 1); // RIGHT

    return neighbors;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Returns -1 if the X and Y position is out of bounds. Otherwise, returns the Grid Index.
//----------------------------------------------------------------------------------------------------
int FInfluenceMap::GetIndexOrNull(int xPos, int yPos) const
{
    const int gridIndex = GetGridIndex(xPos, yPos);

    if (gridIndex >= InfluenceMap.Num() || gridIndex < 0)
		return -1;

    return gridIndex;
}

#pragma region DeprecatedDecay
//void AInfluenceMap::PropagateInfluence()
//{
//    UWorld* pWorld = GetWorld();
//    if (UGameplayStatics::GetGlobalTimeDilation(pWorld) <= 0.f)
//    {
//	    return;
//    }
//
//    //const float start = FPlatformTime::Seconds();
//    //float current = start;
//
//    //// Time sliced update of each cell.
//    //while (current - start < kPropagationTimeSliceAmountS)
//    //{
//    //    float maxInfluence = 0.f;
//    //    float minInfluence = 0.f;
//    //
//    //    auto neighbors = GetNeighbors(CurrentPropagationIndex);
//    //    for (const int neighborIndex : neighbors)
//    //    {
//    //        // If the index is invalid, continue.
//    //        if (neighborIndex == -1)
//    //            continue;
//    //
//    //        const float currentInfluence = PropagationBuffer[neighborIndex];
//    //        const float decay = currentInfluence < 0? Decay : -Decay;
//    //        const float decayedInfluence = currentInfluence * FMath::Exp(decay);
//    //
//    //        maxInfluence = FMath::Max(decayedInfluence, maxInfluence);
//    //        minInfluence = FMath::Min(decayedInfluence, minInfluence);
//    //    }
//
//    //    ++CurrentPropagationIndex;
//
//    //    // If we have finished propagating the entire buffer,
//    //    if (CurrentPropagationIndex >= PropagationBuffer.Num())
//    //    {
//    //        // Swap the Propagation and Render Buffer?
//    //        //Swap(PropagationBuffer, RenderBuffer);
//    //        CurrentPropagationIndex = 0;
//    //    }
//
//    //    current = FPlatformTime::Seconds();
//    //}
//
//    // Set the Modified values?
//    //for (auto pModifier : Modifiers)
//    //{
//    //    const FVector location = pModifier->GetOwner()->GetActorLocation();
//    //    const auto& influenceData = pModifier->GetInfluenceData();
//
//    //    AddModifier(location, influenceData);
//    //}
//
//    //// Decay the stuff?...
//    //for (size_t i = 0; i < EditPropagationBuffer.Num(); ++i)
//    //{
//    //    float maxInfluence = 0.f;
//    //    float minInfluence = 0.f;
//
//    //    /*const float currentInfluence = EditPropagationBuffer[i];
//    //    const float decay = currentInfluence < 0? Decay : -Decay;
//    //	const float decayedInfluence = currentInfluence * FMath::Exp(decay);
//    //    EditPropagationBuffer[i] = decayedInfluence;*/
//
//    //    auto neighbors = GetNeighbors(i);
//    //    for (const int neighborIndex : neighbors)
//    //    {
//    //        // If the index is invalid, continue.
//    //        if (neighborIndex == -1)
//    //            continue;
//
//    //        //const float currentInfluence = InfluenceGrid[neighborIndex].Influence;
//    //        const float currentInfluence = EditPropagationBuffer[neighborIndex];
//    //        //const float decay = currentInfluence < 0? Decay : -Decay;
//    //        //const float decayedInfluence = currentInfluence * Decay;
//    //        const float decayedInfluence = currentInfluence * FMath::Exp(-Decay);
//
//    //        maxInfluence = FMath::Max(decayedInfluence, maxInfluence);
//    //        minInfluence = FMath::Min(decayedInfluence, minInfluence);
//    //    }
//
//    //    EditPropagationBuffer[i] = FMath::Lerp(EditPropagationBuffer[i], maxInfluence + minInfluence, Momentum);
//    //}
//}
#pragma endregion