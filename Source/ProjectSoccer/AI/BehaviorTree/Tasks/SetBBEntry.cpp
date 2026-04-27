// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Tasks/SetBBEntry.h"

#include "BehaviorTree/BlackboardComponent.h"


//----------------------------------------------------------------------------------------------------------------------------------
// Base
//----------------------------------------------------------------------------------------------------------------------------------

void USetBBEntry::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
    SetKeyFilter();
}

EBTNodeResult::Type USetBBEntry::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	SetKeyValue(OwnerComp.GetBlackboardComponent());
    return EBTNodeResult::Succeeded;
}

//----------------------------------------------------------------------------------------------------------------------------------
// Object
//----------------------------------------------------------------------------------------------------------------------------------

USetBBObject::USetBBObject()
{
	NodeName = "Copy Object Value";
}

void USetBBObject::SetKeyFilter()
{
    if (ValidObjectClass)
		ObjectKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USetBBObject, ObjectKey), ValidObjectClass);
}

void USetBBObject::SetKeyValue(UBlackboardComponent* pComponent)
{
    UObject* pCopyObject = pComponent->GetValueAsObject(ObjectKey.SelectedKeyName);
	pComponent->SetValueAsObject(BlackboardKey.SelectedKeyName, pCopyObject);
}

#if WITH_EDITOR
void USetBBObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName propertyName =  PropertyChangedEvent.GetMemberPropertyName();

    if (propertyName == GET_MEMBER_NAME_CHECKED(USetBBObject, ValidObjectClass))
    {
	    ObjectKey.AllowedTypes.Empty();
	    BlackboardKey.AllowedTypes.Empty();

		ObjectKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USetBBObject, ObjectKey), ValidObjectClass);
		BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USetBBObject, BlackboardKey), ValidObjectClass);
    }
}
#endif
