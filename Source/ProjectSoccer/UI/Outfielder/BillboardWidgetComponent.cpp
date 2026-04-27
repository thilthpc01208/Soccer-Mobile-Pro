// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/UI/Outfielder/BillboardWidgetComponent.h"
#include "ProjectSoccer/Entities/GameCamera.h"

UBillboardWidgetComponent::UBillboardWidgetComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
    SetWidgetSpace(EWidgetSpace::World);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Set the Game Camera that this BillboardWidgetComponent will face. If the
///             GameCamera is not null, the Component Tick will be enabled.
//----------------------------------------------------------------------------------------------------
void UBillboardWidgetComponent::SetGameCamera(const TObjectPtr<AGameCamera>& pCamera)
{
    GameCamera = pCamera;

    if (GameCamera)
	    SetComponentTickEnabled(true);
    else
	    SetComponentTickEnabled(false);
}

void UBillboardWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GameCamera)
        return;

    auto cameraRotation = GameCamera->GetCameraRotation();
    cameraRotation.Add(0, 180.0, 0);
    SetWorldRotation(cameraRotation);
}