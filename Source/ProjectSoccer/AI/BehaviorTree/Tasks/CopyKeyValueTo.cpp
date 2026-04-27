// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Tasks/CopyKeyValueTo.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"

void UCopyKeyValueTo::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
    SetKeyFilters();
}

EBTNodeResult::Type UCopyKeyValueTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    CopyFromTo(OwnerComp.GetBlackboardComponent());

    return EBTNodeResult::Succeeded;
}

void UCopyVectorValueTo::SetKeyFilters()
{
	To.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UCopyVectorValueTo, To));
	From.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UCopyVectorValueTo, From));
}

void UCopyVectorValueTo::CopyFromTo(UBlackboardComponent* pComponent)
{
 	const FVector fromValue = pComponent->GetValueAsVector(From.SelectedKeyName);
    pComponent->SetValueAsVector(To.SelectedKeyName, fromValue);
}