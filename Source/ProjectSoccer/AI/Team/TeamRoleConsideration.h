// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TeamRoleConsideration.generated.h"

class AOutfieldAIController;

UCLASS(Abstract)
class PROJECTSOCCER_API UTeamRoleConsideration : public UDataAsset
{
	GENERATED_BODY()

public:
    virtual float Score(AOutfieldAIController* pAiController) { return 1.f; };
};