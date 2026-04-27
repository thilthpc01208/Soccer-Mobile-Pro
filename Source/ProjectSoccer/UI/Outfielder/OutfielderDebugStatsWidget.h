// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OutfielderDebugStatsWidget.generated.h"

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      TODO: Right now, it is hardcoded to show me a set of values.
//      There must be an easier way to add new Stats. I probably need:
//            - Map of UTextBlock* to hold all the stats, mapped to an Interned String
//            - AddStat(const FString name, const float value) to add a new stat
//            - SetStat(const float value, const int index) to set the value of the stat at the given index
//		
///		@brief : 
//----------------------------------------------------------------------------------------------------
UCLASS()
class PROJECTSOCCER_API UOutfielderDebugStatsWidget : public UUserWidget
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "OutfielderStats", meta = (BindWidget))
    class UTextBlock* PressureText;

    UPROPERTY(EditAnywhere, Category = "OutfielderStats", meta = (BindWidget))
    class UTextBlock* ThreatText;

public:
	void SetPressure(const float value) const;
    void SetThreat(const float value) const;
};
