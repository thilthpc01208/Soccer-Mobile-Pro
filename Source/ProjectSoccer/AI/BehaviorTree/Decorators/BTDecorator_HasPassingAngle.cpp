// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Decorators/BTDecorator_HasPassingAngle.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"


bool UBTDecorator_HasPassingAngle::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    const UBlackboardComponent* pBlackboard = OwnerComp.GetBlackboardComponent();
    const AOutfieldCharacter* pOutfielder = Cast<AOutfieldCharacter>(pBlackboard->GetValueAsObject(SelfKey.SelectedKeyName));

    const ATeamManager* pTeamManager = pOutfielder->GetTeamManager();
    AActor* pPlayerOnBall = pTeamManager->GetPlayerInPossessionOrBall();

    if (!pPlayerOnBall || pPlayerOnBall == pOutfielder)
    {
	    return false;
    }

    FCollisionQueryParams traceParams;
    traceParams.AddIgnoredActor(pOutfielder);
    traceParams.AddIgnoredActor(pPlayerOnBall);

    const FVector from = pOutfielder->GetActorLocation();
    const FVector to = pPlayerOnBall->GetActorLocation();

    const bool bHit = pOutfielder->GetWorld()->SweepTestByChannel(from, to, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(18.f), traceParams); // TODO: Ball radius needs to be exposed to the Blackboard.

    return !bHit;
}