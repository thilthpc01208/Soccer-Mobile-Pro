// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Tasks/SteerTo.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"

USteerTo::USteerTo()
{
	SelfKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USteerTo, SelfKey), AOutfieldCharacter::StaticClass());
	PositionKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USteerTo, PositionKey));
	MovementVectorKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USteerTo, MovementVectorKey));
}

EBTNodeResult::Type USteerTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* pBlackboard = OwnerComp.GetBlackboardComponent();
    AOutfieldCharacter* pOutfielder = Cast<AOutfieldCharacter>(pBlackboard->GetValueAsObject(SelfKey.SelectedKeyName));
    
    // Calculate the base direction that we need to move toward.
    const FVector from = pOutfielder->GetActorLocation();
    FVector to = pBlackboard->GetValueAsVector(PositionKey.SelectedKeyName);
    to.Z = from.Z;

    FVector direction = to - from;
    float magnitude = direction.Length(); // Save the magnitude 
    direction.Normalize();

    FVector movementVector = FVector::Zero();

    // If we have arrived, then set our direction to Zero.
    if (magnitude < ArriveDistance)
    {
        direction = FVector::Zero();
    }

    // Otherwise, check for Opponent influence in our current direction.
    else
    {
        static constexpr float kMinInput = 0.0002f;

    	direction = AdjustDirectionForInfluence(pOutfielder, from, direction);
        if (FMath::Abs(direction.Y) > kMinInput)
        {
			movementVector.X = direction.Y; //< 0.f? -1.f : 1.f;
        }

        if (FMath::Abs(direction.X) > kMinInput)
        {
			movementVector.Y = direction.X; //< 0.f? -1.f : 1.f;
        }
    }

	pBlackboard->SetValueAsVector(MovementVectorKey.SelectedKeyName, movementVector);

    return EBTNodeResult::Succeeded;
}

FVector USteerTo::AdjustDirectionForInfluence(const AOutfieldCharacter* pOutfielder, const FVector from, const FVector toDestination) const
{
    const ATeamManager* pTeamManager = pOutfielder->GetTeamManager();
    if (!pTeamManager)
        return toDestination;

    const auto& fieldState = pTeamManager->GetFieldState();
    const auto& influenceMap = fieldState.GetInfluenceMap();
    const TArray<FInfluencePointResult> points = influenceMap.GetInfluencePointsOnArc(from, toDestination, MovementArc);
    const float teamInfluence = pTeamManager->GetInfluenceValue();

    FVector bestDirection = toDestination;
    float bestDot = 0.f;

    // For each point, score whether the point is close to our desired direction and under the opponent influence threshold
    for (size_t i = 0; i < points.Num(); ++i)
    {
        const FVector direction = points[i].Location - from;
        const float dot = toDestination.Dot(direction);

        if (dot > bestDot && points[i].Influence * -teamInfluence < MaxOpponentInfluence)
        {
	        bestDot = dot;
            bestDirection = direction;
        }
    }

    return bestDirection.GetSafeNormal();
}
