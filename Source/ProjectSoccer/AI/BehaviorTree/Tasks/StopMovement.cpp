// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Tasks/StopMovement.h"

#include "BehaviorTree/BlackboardComponent.h"

UStopMovement::UStopMovement()
{
	MovementVectorKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UStopMovement, MovementVectorKey));
}

EBTNodeResult::Type UStopMovement::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* pBlackboard = OwnerComp.GetBlackboardComponent();
    pBlackboard->SetValueAsVector(MovementVectorKey.SelectedKeyName, FVector::Zero());

    return EBTNodeResult::Succeeded;
}