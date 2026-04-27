// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Decorators/BTDecorator_IsNull.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsNull::UBTDecorator_IsNull()
{
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsNull, BlackboardKey), UObject::StaticClass());
}

bool UBTDecorator_IsNull::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* pBlackboard = OwnerComp.GetBlackboardComponent();
    const UObject* pObject = pBlackboard->GetValueAsObject(BlackboardKey.SelectedKeyName);

    return pObject == nullptr;
}