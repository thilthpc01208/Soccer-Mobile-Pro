// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OutfielderInputWidget.generated.h"

//----------------------------------------------------------------------------------------------------
///		@brief : Widget for visualizing an Outfielder's input direction and charge value.
//----------------------------------------------------------------------------------------------------
UCLASS()
class PROJECTSOCCER_API UOutfielderInputWidget : public UUserWidget
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Outfielder", meta = (BindWidget))
    class UCanvasPanel* MainCanvas;

    UPROPERTY(EditAnywhere, Category = "Outfielder", meta = (BindWidget))
    class UImage* DirectionRingImage;

    UPROPERTY(EditAnywhere, Category = "Outfielder", meta = (BindWidget))
    class UImage* DirectionArrowImage;

    UPROPERTY(EditAnywhere, Category = "Outfielder", meta = (BindWidget))
    class UImage* ChargeRingImage;

public:
    void HideCompletely();
    void ShowDirection(const FVector& startDirection);
    void HideDirection();
    void SetDirection(const FVector& direction);

    void ShowCharge(const float startNormalizedCharge);
    void HideCharge();
    void SetCharge(float normalizedCharge);
};
