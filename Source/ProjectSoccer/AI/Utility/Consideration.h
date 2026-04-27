// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UtilityScoreProvider.h"
#include "Engine/DataAsset.h"
#include "Consideration.generated.h"

UCLASS(Abstract)
class PROJECTSOCCER_API UConsideration : public UDataAsset
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditDefaultsOnly, Category = Consideration)
    bool bInvertResult = false;
};