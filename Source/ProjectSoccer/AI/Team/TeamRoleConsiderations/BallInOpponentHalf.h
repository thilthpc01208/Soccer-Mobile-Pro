// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectSoccer/AI/Team/TeamRoleScoreProvider.h"
#include "ProjectSoccer/AI/Utility/BooleanConsideration.h"
#include "BallInOpponentHalf.generated.h"

UCLASS()
class PROJECTSOCCER_API UBallInOpponentHalf : public UBooleanConsideration, public ITeamRoleScoreProvider
{
	GENERATED_BODY()

public:
    virtual float GetScore(const FTeamRoleContext& context) override;
};
