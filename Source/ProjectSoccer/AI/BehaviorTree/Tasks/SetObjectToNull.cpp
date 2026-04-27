// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Tasks/SetObjectToNull.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type USetObjectToNull::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* pBlackboard = OwnerComp.GetBlackboardComponent();
    pBlackboard->SetValueAsObject(ObjectKey.SelectedKeyName, nullptr);

    return EBTNodeResult::Succeeded;
}

#if WITH_EDITOR
void USetObjectToNull::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName propertyName =  PropertyChangedEvent.GetMemberPropertyName();

    if (propertyName == GET_MEMBER_NAME_CHECKED(USetObjectToNull, ValidObjectClass))
    {
	    ObjectKey.AllowedTypes.Empty();
		ObjectKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USetObjectToNull, ObjectKey), ValidObjectClass);
    }
}
#endif
