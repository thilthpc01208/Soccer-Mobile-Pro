// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Tasks/SetOutfielderBasedOnProximity.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"

USetOutfielderBasedOnProximity::USetOutfielderBasedOnProximity()
{
    SelfKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USetOutfielderBasedOnProximity, SelfKey), AOutfieldCharacter::StaticClass());
    OutfielderKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USetOutfielderBasedOnProximity, OutfielderKey), AOutfieldCharacter::StaticClass());
	PositionKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(USetOutfielderBasedOnProximity, PositionKey));
}

EBTNodeResult::Type USetOutfielderBasedOnProximity::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* pBlackboard = OwnerComp.GetBlackboardComponent();
    AOutfieldCharacter* pOutfielder = Cast<AOutfieldCharacter>(pBlackboard->GetValueAsObject(SelfKey.SelectedKeyName));
    ATeamManager* pTeamManager = pOutfielder->GetTeamManager();
    const FVector targetLocation = pBlackboard->GetValueAsVector(PositionKey.SelectedKeyName);

    // Find the closest outfielder to the target location.
    AOutfieldCharacter* pClosest = pTeamManager->GetClosestOutfielderToLocation(targetLocation);

    // Set the Outfielder Key.
    pBlackboard->SetValueAsObject(OutfielderKey.SelectedKeyName, pClosest);

    return EBTNodeResult::Succeeded;
}