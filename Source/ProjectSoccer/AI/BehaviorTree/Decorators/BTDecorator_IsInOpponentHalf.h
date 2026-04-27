// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsInOpponentHalf.generated.h"

UCLASS()
class PROJECTSOCCER_API UBTDecorator_IsInOpponentHalf : public UBTDecorator
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector SelfKey;

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector BlackboardKey;

public:
    UBTDecorator_IsInOpponentHalf();
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private:
    //virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
