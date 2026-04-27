// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Decorators/BTDecorator_WithinRange.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"

UBTDecorator_WithinRange::UBTDecorator_WithinRange()
{
	NodeName = "Within Range";

    SelfKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_WithinRange, SelfKey), AOutfieldCharacter::StaticClass());
}

bool UBTDecorator_WithinRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* pBlackboard = OwnerComp.GetBlackboardComponent();
    const AOutfieldCharacter* pOutfielder = Cast<AOutfieldCharacter>(pBlackboard->GetValueAsObject(SelfKey.SelectedKeyName));
    if (!pOutfielder)
        return false;

    const ATeamManager* pTeamManager = pOutfielder->GetTeamManager();
    const AActor* pBallOrOutfielder = pTeamManager->GetPlayerInPossessionOrBall();


    const FVector outFielderLocation = pOutfielder->GetActorLocation();
    const FVector otherLocation = pBallOrOutfielder->GetActorLocation();

    const float min = FMath::Min(Min, Max);
    const float max = FMath::Max(Min, Max);

    const FVector to = otherLocation - outFielderLocation;
    const float sqrLength = to.SquaredLength();

    return min * min < sqrLength && sqrLength < max * max;
}

FVector UBTDecorator_WithinRange::GetLocationFromKey(const UBlackboardComponent* pBlackboard, const FBlackboardKeySelector key) const
{
    if (key.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
    {
	    return pBlackboard->GetValueAsVector(key.SelectedKeyName);;
    }

    const AActor* pActor = Cast<AActor>(pBlackboard->GetValueAsObject(key.SelectedKeyName));
    return pActor->GetActorLocation();
}
