// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_IsValidLocation.generated.h"

UCLASS()
class PROJECTSOCCER_API UBTDecorator_IsValidLocation : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

public:
    UBTDecorator_IsValidLocation();

    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
