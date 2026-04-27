// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Tasks/SetPositionToPlayerOrBall.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"

USetPositionToPlayerOrBall::USetPositionToPlayerOrBall()
{
	SelfKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USetPositionToPlayerOrBall, SelfKey), AOutfieldCharacter::StaticClass());
	PositionKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USetPositionToPlayerOrBall, PositionKey));
}

EBTNodeResult::Type USetPositionToPlayerOrBall::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* pBlackboard = OwnerComp.GetBlackboardComponent();
    const AOutfieldCharacter* pOutfielder = Cast<AOutfieldCharacter>(pBlackboard->GetValueAsObject(SelfKey.SelectedKeyName));
    const ATeamManager* pTeamManager = pOutfielder->GetTeamManager();

    const AActor* pPlayerOrBall = pTeamManager->GetPlayerInPossessionOrBall();
    const FVector playerOrBallLocation = pPlayerOrBall->GetActorLocation();

    pBlackboard->SetValueAsVector(PositionKey.SelectedKeyName, playerOrBallLocation);

    return EBTNodeResult::Succeeded;
}
