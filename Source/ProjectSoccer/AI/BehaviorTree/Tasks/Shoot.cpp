// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Tasks/Shoot.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"

UShoot::UShoot()
{
	SelfKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UShoot, SelfKey), AOutfieldCharacter::StaticClass());
	MovementVectorKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UShoot, MovementVectorKey));
}

EBTNodeResult::Type UShoot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* pBlackboard = OwnerComp.GetBlackboardComponent();
    AOutfieldCharacter* pOutfielder = Cast<AOutfieldCharacter>(pBlackboard->GetValueAsObject(SelfKey.SelectedKeyName));

    // Get the current MovementVector.
    const FVector movementVector3D = pBlackboard->GetValueAsVector(MovementVectorKey.SelectedKeyName);

	// [TODO]: I need to have the behavior not on just the movement vector, but an input vector.
	
    FVector2D movementVector = FVector2D(movementVector3D.X, movementVector3D.Y);
    movementVector.Normalize();
	
    // TODO: This should be StartShotOrTackle, then in an update, actually shoot when the target shot power is met.
    //      The shot power is charged on the Enemy.
    pOutfielder->Shoot(movementVector);

    return EBTNodeResult::Succeeded;
}