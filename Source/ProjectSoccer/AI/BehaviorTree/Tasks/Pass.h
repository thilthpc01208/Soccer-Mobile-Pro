// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Pass.generated.h"

UCLASS()
class PROJECTSOCCER_API UPass : public UBTTaskNode
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector SelfKey;

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector TeammateKey;

public:
    UPass();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};