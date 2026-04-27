// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Command.generated.h"

UCLASS(Abstract, Blueprintable)
class PROJECTSOCCER_API UCommand : public UObject
{
    GENERATED_BODY()

public:
	virtual ~UCommand() override = default;
    virtual void Execute(){}
};
