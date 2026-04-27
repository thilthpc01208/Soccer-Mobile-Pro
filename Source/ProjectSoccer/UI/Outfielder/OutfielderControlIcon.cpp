// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/UI/Outfielder/OutfielderControlIcon.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UOutfielderControlIcon::SetPlayerIcon(const int index)
{
    check(index >= 0 && index < PlayerColors.Num());

    const FString playerText = "P" + FString::FromInt(index + 1);
    IconText->SetText(FText::FromString(playerText));
    IconText->SetColorAndOpacity(PlayerColors[index]);
    IconText->SetVisibility(ESlateVisibility::Visible);

    ArrowIcon->SetColorAndOpacity(PlayerColors[index]);
}

void UOutfielderControlIcon::SetCPUIcon() const
{
    //const FString cpuText = "CPU" + FString::FromInt(index + 1);
    IconText->SetVisibility(ESlateVisibility::Hidden);
    //IconText->SetText(FText::FromString("CPU"));
    //IconText->SetColorAndOpacity(CPUColor);

    ArrowIcon->SetColorAndOpacity(CPUColor);
}