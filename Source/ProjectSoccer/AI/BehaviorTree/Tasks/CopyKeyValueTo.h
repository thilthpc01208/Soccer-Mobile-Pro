// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CopyKeyValueTo.generated.h"

UCLASS(Abstract)
class PROJECTSOCCER_API UCopyKeyValueTo : public UBTTaskNode
{
	GENERATED_BODY()

protected:
    /** Value we are going to copy. */
    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector From;

    /** Value we are going to write the copied value to. */
    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector To;

public:
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    virtual void CopyFromTo([[maybe_unused]] UBlackboardComponent* pComponent) {}
    virtual void SetKeyFilters() {}

};

UCLASS()
class PROJECTSOCCER_API UCopyVectorValueTo final : public UCopyKeyValueTo
{
    GENERATED_BODY()

    virtual void CopyFromTo(UBlackboardComponent* pComponent) override;
    virtual void SetKeyFilters() override;
};
