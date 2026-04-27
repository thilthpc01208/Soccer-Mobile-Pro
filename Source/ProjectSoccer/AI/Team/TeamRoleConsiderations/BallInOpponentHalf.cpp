// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/Team/TeamRoleConsiderations/BallInOpponentHalf.h"

#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"

float UBallInOpponentHalf::GetScore(const FTeamRoleContext& context)
{
    const AActor* pBallOrPlayer = context.pTeamManager->GetPlayerInPossessionOrBall();
    const FVector ballLocation = pBallOrPlayer->GetActorLocation();

    const float dot = FVector::DotProduct(ballLocation, context.pTeamManager->GetActorForwardVector());
    const bool bResult = dot > 0.f;

    return GetTrueFalseScore(bResult);
}