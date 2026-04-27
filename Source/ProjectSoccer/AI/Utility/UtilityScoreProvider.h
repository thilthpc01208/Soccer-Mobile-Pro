// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

template<typename TContextType>
class PROJECTSOCCER_API IUtilityScoreProvider
{
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual ~IUtilityScoreProvider() = default;

    virtual float GetScore(TContextType context) = 0;
};
