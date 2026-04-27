// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SetGoaliePosition.generated.h"

UCLASS()
class PROJECTSOCCER_API USetGoaliePosition : public UBTTaskNode
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector SelfKey;

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector PositionKey;

    UPROPERTY(EditAnywhere, Category = Blackboard)
    float MinDistanceFromGoal = 100.f;

public:
    USetGoaliePosition();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
