// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SetPositionToPlayerOrBall.generated.h"

UCLASS()
class PROJECTSOCCER_API USetPositionToPlayerOrBall : public UBTTaskNode
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector SelfKey;

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector PositionKey;

public:
    USetPositionToPlayerOrBall();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
