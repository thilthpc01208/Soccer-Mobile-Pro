// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ProjectSoccer/Entities/Field/InfluenceMap.h"
#include "SteerTo.generated.h"

UCLASS()
class PROJECTSOCCER_API USteerTo : public UBTTaskNode
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector SelfKey;

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector PositionKey;

    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector MovementVectorKey;

	/**The Radius determines the distance that we will check ahead of the Agent. The arc degrees determines the amount the agent will be able to
	turn when trying to avoid opponent influence.*/
    UPROPERTY(EditAnywhere, Category = Movement)
    FInfluenceArcQueryParams MovementArc;

    UPROPERTY(EditAnywhere, Category = Movement, meta = (ClampMin = 0.f, ClampMax = 1.f))
    float MaxOpponentInfluence = 0.5f;

    UPROPERTY(EditAnywhere, Category = Movement, meta = (ClampMin = 0.f))
    float ArriveDistance = 5.f;

public:
    USteerTo();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
    FVector AdjustDirectionForInfluence(const AOutfieldCharacter* pOutfielder, const FVector from, const FVector toDestination) const;
};
