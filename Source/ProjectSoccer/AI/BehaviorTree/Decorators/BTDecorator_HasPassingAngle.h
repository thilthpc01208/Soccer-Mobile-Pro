// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_HasPassingAngle.generated.h"

UCLASS()
class PROJECTSOCCER_API UBTDecorator_HasPassingAngle : public UBTDecorator
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector SelfKey;

public:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
