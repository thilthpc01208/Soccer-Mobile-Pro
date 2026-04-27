// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Consideration.h"
#include "Engine/DataAsset.h"
#include "BooleanConsideration.generated.h"

UCLASS(Abstract)
class PROJECTSOCCER_API UBooleanConsideration : public UConsideration
{
	GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = Consideration, meta = (ClampMin = 0.f, ClampMax = 1.f))
    float TrueScore = 1.f;

    UPROPERTY(EditDefaultsOnly, Category = Consideration, meta = (ClampMin = 0.f, ClampMax = 1.f))
    float FalseScore = 0.f;

protected:
    float GetTrueFalseScore(const bool bResult) const;
};
