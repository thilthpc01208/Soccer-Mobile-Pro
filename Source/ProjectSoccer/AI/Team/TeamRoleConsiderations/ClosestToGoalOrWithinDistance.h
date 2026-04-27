// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectSoccer/AI/Team/TeamRoleScoreProvider.h"
#include "ProjectSoccer/AI/Utility/BooleanConsideration.h"
#include "ClosestToGoalOrWithinDistance.generated.h"

UCLASS()
class PROJECTSOCCER_API UClosestToGoalOrWithinDistance : public UBooleanConsideration, public ITeamRoleScoreProvider
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    float MinDistanceToGoal = 300.f;

public:
    virtual float GetScore(const FTeamRoleContext& context) override;
};
