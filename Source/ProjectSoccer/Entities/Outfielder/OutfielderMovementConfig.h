// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OutfielderMovementConfig.generated.h"

UCLASS()
class PROJECTSOCCER_API UOutfielderMovementConfig : public UDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Ground")
    float MaxSpeed = 750.f;

    /*UPROPERTY(EditAnywhere)
    float MaxTurnSpeed;

    UPROPERTY(EditAnywhere)
    float MaxAirControlSpeed;*/

	//----------------------------------------------------------------------------------------------------
	///		@brief : The Maximum Distance from the Outfielder's Location that the Outfielder's Pass Target
    ///              can be based on the ratio of CurrentSpeed / MaxSpeed.
	//----------------------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Ball Control")
    float MaxPassLeadDistance = 300.f;

    // TODO: Curve for the Pass Lead Distance?

    UPROPERTY(EditAnywhere, Category = "Ball Control")
    float MaxPossessionTargetDistance = 300.f;

};
