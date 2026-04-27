// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectSoccer/AI/Team/TeamRoleConsiderations/ClosestToBallOrPlayerInPossession.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"

float UClosestToBallOrPlayerInPossession::GetScore(const FTeamRoleContext& context)
{
    const AActor* pBallOrPlayer = context.pTeamManager->GetPlayerInPossessionOrBall();
    const TObjectPtr<AOutfieldCharacter> pClosestPlayer = context.pTeamManager->GetClosestOutfielderToLocation(pBallOrPlayer->GetActorLocation());

    const bool bResult = pClosestPlayer == context.pOutfielder;
    return GetTrueFalseScore(bResult);
}
