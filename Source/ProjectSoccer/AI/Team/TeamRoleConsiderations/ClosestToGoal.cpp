// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/Team/TeamRoleConsiderations/ClosestToGoal.h"

#include "ProjectSoccer/Entities/Field/GoalTrigger.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"

float UClosestToGoal::GetScore(const FTeamRoleContext& context)
{
    const AGoalTrigger* pGoal = context.pTeamManager->GetOpponentGoal();
    const TObjectPtr<AOutfieldCharacter> pClosestPlayer = context.pTeamManager->GetClosestOutfielderToLocation(pGoal->GetActorLocation());

    const bool bResult = pClosestPlayer == context.pOutfielder;
    return GetTrueFalseScore(bResult);
}