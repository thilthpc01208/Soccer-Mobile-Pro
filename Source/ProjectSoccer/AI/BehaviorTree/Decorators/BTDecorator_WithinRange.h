// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_WithinRange.generated.h"

UCLASS()
class PROJECTSOCCER_API UBTDecorator_WithinRange : public UBTDecorator
{
	GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector SelfKey;

    UPROPERTY(EditAnywhere, Category = Blackboard, meta = (ClampMin = 0.f))
    float Min;

    UPROPERTY(EditAnywhere, Category = Blackboard, meta = (ClampMin = 0.f))
    float Max;

public:
    UBTDecorator_WithinRange();
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private:
    FVector GetLocationFromKey(const UBlackboardComponent* pBlackboard, const FBlackboardKeySelector key) const;
};
