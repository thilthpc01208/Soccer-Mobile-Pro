// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/Utility/BooleanConsideration.h"

float UBooleanConsideration::GetTrueFalseScore(const bool bResult) const
{
	float result = bResult? TrueScore : FalseScore;

    if (bInvertResult)
        result = 1.f - result;

    return result;
}
