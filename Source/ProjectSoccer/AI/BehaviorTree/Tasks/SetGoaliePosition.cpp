// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Tasks/SetGoaliePosition.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectSoccer/Entities/Field/GoalTrigger.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"

USetGoaliePosition::USetGoaliePosition()
{
    SelfKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USetGoaliePosition, SelfKey), AOutfieldCharacter::StaticClass());
	PositionKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USetGoaliePosition, PositionKey));
}

EBTNodeResult::Type USetGoaliePosition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* pBlackboard = OwnerComp.GetBlackboardComponent();
    AOutfieldCharacter* pOutfielder = Cast<AOutfieldCharacter>(pBlackboard->GetValueAsObject(SelfKey.SelectedKeyName));
    const ATeamManager* pTeamManager = pOutfielder->GetTeamManager();
    const AGoalTrigger* pTeamGoal = pTeamManager->GetTeamGoal();

    const FVector outfielderPosition = pTeamManager->GetPlayerInPossessionOrBall()->GetActorLocation();
    const auto& postPositions = pTeamGoal->GetPostPositions();

    // Calculate the center position from the two posts and the goal.
    FVector centroid = outfielderPosition + postPositions[0] + postPositions[1];
    centroid /= 3.f;

    const FVector goalCenter = pTeamGoal->GetActorLocation();
    FVector goalToCentroid = centroid - goalCenter;;
    const float distance = goalToCentroid.Length();
    goalToCentroid.Normalize();

    // Make sure that the position is far enough from the goal.
    if (distance < MinDistanceFromGoal)
    {
	    centroid = goalCenter + (goalToCentroid * MinDistanceFromGoal);
    }

    pBlackboard->SetValueAsVector(PositionKey.SelectedKeyName, centroid);
    return EBTNodeResult::Succeeded;
}
