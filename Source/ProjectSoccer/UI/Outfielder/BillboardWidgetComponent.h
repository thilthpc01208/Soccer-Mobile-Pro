// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "BillboardWidgetComponent.generated.h"

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : The BillboardWidgetComponent is a WidgetComponent that always faces the Game Camera.
//----------------------------------------------------------------------------------------------------
UCLASS()
class PROJECTSOCCER_API UBillboardWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<class AGameCamera> GameCamera = nullptr;

public:
    UBillboardWidgetComponent();
    void SetGameCamera(const TObjectPtr<AGameCamera>& pCamera);

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
