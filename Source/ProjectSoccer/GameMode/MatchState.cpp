// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectSoccer/GameMode/MatchState.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"

FGameplayTag FMatchState::GetTeamInPossessionTag() const
{
    if (OutfielderInPossession)
    {
        return OutfielderInPossession->GetTeamTag();
    }

    return FGameplayTag::EmptyTag;
}


//----------------------------------------------------------------------------------------------------
///		@brief : Reset the Match State to the default values.
//----------------------------------------------------------------------------------------------------
void FMatchState::Reset()
{
    // FieldState.Reset();
    OutfielderInPossession = nullptr;
    GoalsScored.Empty();
    HomeTeamScore = 0;
    AwayTeamScore = 0;
}