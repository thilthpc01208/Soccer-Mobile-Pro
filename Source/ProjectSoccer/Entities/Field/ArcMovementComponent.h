// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"
#include "ArcMovementComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogArcMovementComponent, Log, All);
DECLARE_EVENT(UArcMovementComponent, FOnArcMovementFinished);

struct FMoveAlongArcParams
{
    FVector StartLocation{};
    FVector TargetLocation{};
    FVector InitialVelocity{};
    FVector ArcAxis{};              // A Direction perpendicular to the direction from start to target.
    class UCurveFloat* VelocityCurve = nullptr; // Curve Describing the Velocity of the Move over time.
    float MaxArcHeight = 0.f;       // The highest point of the arc from Max(StartLocation.Z, TargetLocation.Z), in direction ArcAxis.
    float MaxHeightOffset = 0.f;    // The highest point of the arc from Max(StartLocation.Z, TargetLocation.Z).
    float Speed = 1.f;			    // The speed of the movement.
};

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      I was thinking the InterpToMovementComponent would work, but I believe I need to create some
//		
///		@brief : This will move the updated Component along an arc path, but stop updating once the
///             EndLocation has been , or the UpdatedComponent has
//----------------------------------------------------------------------------------------------------
UCLASS()
class PROJECTSOCCER_API UArcMovementComponent : public UMovementComponent
{
	GENERATED_BODY()

    DECLARE_DELEGATE_RetVal(bool, FOnArcMovementFinishedOrInterrupted);

private:
    UPROPERTY()
    FVector StartLocation{};
    FVector InitialVelocity{};
    FVector Acceleration{};
    //FVector LastLocation{};
    //FVector EndLocation{};
	//float Speed         = 0.f;
    //float MaxHeight     = 0.f;
    //float UpAngleRad    = 0.f;
    //float RightAngleRad = 0.f;
    float Duration      = 0.f;
    float CurrentTime   = 0.f;
    bool bIsSimulating  = false;

    FOnArcMovementFinished OnArcMovementFinishedEvent;
    FOnArcMovementFinishedOrInterrupted OnArcMovementFinishedOrInterruptedDelegate;

public:
    UArcMovementComponent();

    virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
    void CancelArc();
    bool BeginMoveAlongArc(const FMoveAlongArcParams& params, float& duration);

    FOnArcMovementFinished& OnArcMovementFinished() { return OnArcMovementFinishedEvent; }
	FOnArcMovementFinishedOrInterrupted& OnArcMovementFinishedOrInterrupted();

private:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void EndArcSimulation();
};
