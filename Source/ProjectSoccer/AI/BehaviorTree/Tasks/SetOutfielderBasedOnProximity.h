// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SetOutfielderBasedOnProximity.generated.h"

UCLASS()
class PROJECTSOCCER_API USetOutfielderBasedOnProximity : public UBTTaskNode
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector SelfKey;

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector PositionKey;

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector OutfielderKey;

public:
    USetOutfielderBasedOnProximity();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
