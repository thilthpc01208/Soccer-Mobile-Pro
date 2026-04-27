// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Tasks/Pass.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"

UPass::UPass()
{
	SelfKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UPass, SelfKey), AOutfieldCharacter::StaticClass());
	TeammateKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UPass, TeammateKey), AOutfieldCharacter::StaticClass());
}

EBTNodeResult::Type UPass::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* pBlackboard = OwnerComp.GetBlackboardComponent();
    AOutfieldCharacter* pOutfielder = Cast<AOutfieldCharacter>(pBlackboard->GetValueAsObject(SelfKey.SelectedKeyName));
    AOutfieldCharacter* pTeammate = Cast<AOutfieldCharacter>(pBlackboard->GetValueAsObject(TeammateKey.SelectedKeyName));

    if (pTeammate == nullptr || pOutfielder == pTeammate)
    {
	    return EBTNodeResult::Failed;
    }

    pOutfielder->PassToTeammate(pTeammate);

    return EBTNodeResult::Succeeded;
}