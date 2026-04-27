// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/Team/TeamRoleConsiderations/PlayerIsInPossession.h"

#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"

float UPlayerIsInPossession::GetScore(const FTeamRoleContext& context)
{
    const AActor* pPlayerInPossession = context.pTeamManager->GetPlayerInPossession();
    if (!pPlayerInPossession)
        return GetTrueFalseScore(false);
	
    const bool bResult = context.pOutfielder == pPlayerInPossession;

    return GetTrueFalseScore(bResult);
}