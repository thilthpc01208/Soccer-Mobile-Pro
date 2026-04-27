// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Decorators/BTDecorator_IsValidLocation.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsValidLocation::UBTDecorator_IsValidLocation()
{
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsValidLocation, BlackboardKey));
}

bool UBTDecorator_IsValidLocation::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    const UBlackboardComponent* pBlackboardComponent = OwnerComp.GetBlackboardComponent();

	const FVector vectorValue = pBlackboardComponent->GetValueAsVector(BlackboardKey.SelectedKeyName);
    return FAISystem::IsValidLocation(vectorValue);
}
