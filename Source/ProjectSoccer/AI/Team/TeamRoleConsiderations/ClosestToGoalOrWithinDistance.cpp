// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/Team/TeamRoleConsiderations/ClosestToGoalOrWithinDistance.h"

#include "ProjectSoccer/Entities/Field/GoalTrigger.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"

float UClosestToGoalOrWithinDistance::GetScore(const FTeamRoleContext& context)
{
    const AGoalTrigger* pGoal = context.pTeamManager->GetTeamGoal();
    const FVector goalLocation = pGoal->GetActorLocation();
    const TObjectPtr<AOutfieldCharacter> pClosestPlayer = context.pTeamManager->GetClosestOutfielderToLocation(goalLocation);

    if (pClosestPlayer == context.pOutfielder)
        return GetTrueFalseScore(true);

    const FVector outfielderLocation = context.pOutfielder->GetActorLocation();
    const float distanceToGoal = (outfielderLocation - goalLocation).SquaredLength();

    // Return true if the outfielder is close enough to the goal anyway.
    const bool bResult = distanceToGoal < (MinDistanceToGoal * MinDistanceToGoal);
    return GetTrueFalseScore(bResult);
}