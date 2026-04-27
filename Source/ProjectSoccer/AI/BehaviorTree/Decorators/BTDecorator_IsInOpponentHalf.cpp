// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Decorators/BTDecorator_IsInOpponentHalf.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"

UBTDecorator_IsInOpponentHalf::UBTDecorator_IsInOpponentHalf()
{
    NodeName = "Location in Opponent Half";

    SelfKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsInOpponentHalf, SelfKey), AOutfieldCharacter::StaticClass());

    BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsInOpponentHalf, BlackboardKey));
    BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsInOpponentHalf, BlackboardKey), AActor::StaticClass());
}

void UBTDecorator_IsInOpponentHalf::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

    UBlackboardData* BBAsset = GetBlackboardAsset();
    SelfKey.ResolveSelectedKey(*BBAsset);
	BlackboardKey.ResolveSelectedKey(*BBAsset);
}

bool UBTDecorator_IsInOpponentHalf::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    const UBlackboardComponent* pBlackboard = OwnerComp.GetBlackboardComponent();
    const AOutfieldCharacter* pOutfielder = Cast<AOutfieldCharacter>(pBlackboard->GetValueAsObject(SelfKey.SelectedKeyName));
    const ATeamManager* pTeamManager = pOutfielder->GetTeamManager();
    
    bool bInHalf = false;
    if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
    {
        const FVector location = pBlackboard->GetValueAsVector(BlackboardKey.SelectedKeyName);
        bInHalf = !pTeamManager->LocationInTeamHalf(location);
    }

    else
    {
        const AActor* pActor = Cast<AActor>(pBlackboard->GetValueAsObject(BlackboardKey.SelectedKeyName));
        if (!pActor)
            return false;

		bInHalf = !(pTeamManager->LocationInTeamHalf(pActor->GetActorLocation()));
	    
    }

    return bInHalf;
}
