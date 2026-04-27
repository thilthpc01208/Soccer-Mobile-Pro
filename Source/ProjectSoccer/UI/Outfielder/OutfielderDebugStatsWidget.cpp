// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/UI/Outfielder/OutfielderDebugStatsWidget.h"
#include "Components/TextBlock.h"

void UOutfielderDebugStatsWidget::SetPressure(const float value) const
{
    PressureText->SetText(FText::FromString(FString::Printf(TEXT("Pressure: %.2f"), value)));
}

void UOutfielderDebugStatsWidget::SetThreat(const float value) const
{
    ThreatText->SetText(FText::FromString(FString::Printf(TEXT("Threat: %.2f"), value)));
}