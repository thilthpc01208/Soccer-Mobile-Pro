// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SetBBEntry.generated.h"

UCLASS(Abstract)
class PROJECTSOCCER_API USetBBEntry : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	UPROPERTY(Category = Blackboard, EditAnywhere)
    FBlackboardKeySelector BlackboardKey;

public:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    virtual void SetKeyValue([[maybe_unused]] UBlackboardComponent* pComponent) {}
    virtual void SetKeyFilter() {}
};

//----------------------------------------------------------------------------------------------------------------------------------
// Task to Copy an Object Value of another key to the BlackboardKey in the Blackboard.
//----------------------------------------------------------------------------------------------------------------------------------
UCLASS()
class PROJECTSOCCER_API USetBBObject : public USetBBEntry
{
	GENERATED_BODY()

    UPROPERTY(Category=Blackboard, EditAnywhere, meta= (DisplayAfter = BlackboardKey))
    TSubclassOf<UObject> ValidObjectClass;

    UPROPERTY(Category=Blackboard, EditAnywhere, meta= (DisplayAfter = BlackboardKey))
    FBlackboardKeySelector ObjectKey;

public:
    USetBBObject();

private:
    virtual void SetKeyValue(UBlackboardComponent* pComponent) override;
    virtual void SetKeyFilter() override;

    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
