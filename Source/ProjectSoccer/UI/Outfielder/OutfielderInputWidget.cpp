// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectSoccer/UI/Outfielder/OutfielderInputWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"

void UOutfielderInputWidget::HideCompletely()
{
    HideDirection();
    HideCharge();
}

void UOutfielderInputWidget::ShowDirection(const FVector& startDirection)
{
	//DirectionRingImage->SetVisibility(ESlateVisibility::Visible);
	//DirectionArrowImage->SetVisibility(ESlateVisibility::Visible);
    //SetDirection(startDirection);
}

void UOutfielderInputWidget::HideDirection()
{
    //DirectionRingImage->SetVisibility(ESlateVisibility::Hidden);
    //DirectionArrowImage->SetVisibility(ESlateVisibility::Hidden);
}

//----------------------------------------------------------------------------------------------------
///		@brief : Set the Direction that the Outfielder is currently aiming towards.
/// 	@param direction : The direction vector that the Outfielder is aiming towards. This should NOT
///          be normalized! We are using the Length of the vector to determine the size of the direction
///          arrow.
//----------------------------------------------------------------------------------------------------
void UOutfielderInputWidget::SetDirection(const FVector& direction)
{
    //const float length = direction.Length(); // Non-normalized length.
    //DirectionArrowImage->SetRenderScale(FVector2D(length, length));
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Show the Charge display for the Outfielder.
///		@param startNormalizedCharge : Beginning charge value, normalized between 0 and 1.
//----------------------------------------------------------------------------------------------------
void UOutfielderInputWidget::ShowCharge(const float startNormalizedCharge)
{
    DirectionRingImage->SetVisibility(ESlateVisibility::Visible);
    ChargeRingImage->SetVisibility(ESlateVisibility::Visible);
    SetCharge(startNormalizedCharge);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Hide the Charge display for the Outfielder.
//----------------------------------------------------------------------------------------------------
void UOutfielderInputWidget::HideCharge()
{
    DirectionRingImage->SetVisibility(ESlateVisibility::Hidden);
    ChargeRingImage->SetVisibility(ESlateVisibility::Hidden);
}

//----------------------------------------------------------------------------------------------------
///		@brief : Set the Charge value for the Outfielder, normalized between 0 and 1.
//----------------------------------------------------------------------------------------------------
void UOutfielderInputWidget::SetCharge(float normalizedCharge)
{
    ChargeRingImage->SetRenderScale(FVector2D(normalizedCharge, normalizedCharge));
}