// Fill out your copyright notice in the Description page of Project Settings.

#include "Formation.h"

#include "ProjectSoccer/GameMode/ProjectSoccerGameMode.h"

UFormation::UFormation()
{
	Points.SetNumZeroed(AProjectSoccerGameMode::kMaxOutfielderCount);
}

void UGameFormation::SetFormationData(const TObjectPtr<UFormation> pFormation)
{
    ClearFormationData();

	const auto& points = pFormation->GetPoints();
    for (const auto& point : points)
    {
	    Points.Add(point);
        //PointRotations.Add(point.Rotation);
        PointAssignedStatuses.Add(false);
    }
}

void UGameFormation::SetFormationRotation(const FRotator rotation)
{
    FormationRotation = rotation;

	for (auto& point : Points)
	{
		point = rotation.RotateVector(point);
	}
}

FVector UGameFormation::GetLocationOfPoint(const size_t positionIndex) const
{
	check(Points.Num() > positionIndex);
    //return FormationRotation.RotateVector(Points[pointIndex]);
    return Points[positionIndex];
}

FVector UGameFormation::GetClosestPointTo(const FVector worldLocation) const
{
    size_t closest = std::numeric_limits<size_t>::max();
    float closestSquared = std::numeric_limits<float>::max();

	for (size_t i = 0; i < Points.Num(); ++i)
	{
		//const FFormationPoint& point = Points[i];

        const FVector to = Points[i] - worldLocation;
        const float squaredLength = to.SquaredLength();

        if (squaredLength < closestSquared)
        {
	        closestSquared = squaredLength;
            closest = i;
        }
	}

    return Points[closest];
}

void UGameFormation::SetPositionAssigned(const size_t positionIndex, const bool bIsAssigned)
{
	check(PointAssignedStatuses.Num() > positionIndex);
    PointAssignedStatuses[positionIndex] = bIsAssigned; 
}

bool UGameFormation::GetPositionAssigned(const size_t positionIndex) const
{
	check(PointAssignedStatuses.Num() > positionIndex);
    return PointAssignedStatuses[positionIndex];
}

void UGameFormation::ClearAllAssignments()
{
	for (size_t i = 0; i < PointAssignedStatuses.Num(); ++i)
	{
		PointAssignedStatuses[i] = false;
	}
}

void UGameFormation::SetDebugColors(const FColor activeColor, const FColor inactiveColor)
{
	ActiveColor = activeColor;
    InactiveColor = inactiveColor;
}

FColor UGameFormation::GetDebugColor(const size_t pointIndex) const
{
	check(Points.Num() > pointIndex);
    return PointAssignedStatuses[pointIndex] ? ActiveColor : InactiveColor;
}

void UGameFormation::ClearFormationData()
{
	Points.Empty();
    //PointRotations.Empty();
    PointAssignedStatuses.Empty();
}
