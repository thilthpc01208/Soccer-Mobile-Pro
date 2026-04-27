// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OutfielderControlIcon.generated.h"

UCLASS()
class PROJECTSOCCER_API UOutfielderControlIcon : public UUserWidget
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "PlayerIcon", meta = (BindWidget))
    class UImage* ArrowIcon;

    UPROPERTY(EditAnywhere, Category = "PlayerIcon", meta = (BindWidget))
    class UTextBlock* IconText;

    UPROPERTY(EditAnywhere, Category = "PlayerIcon|Colors")
    FLinearColor CPUColor = FLinearColor(0.6f, 0.6f, 0.6f, 1.f);

    UPROPERTY(EditAnywhere, Category = "PlayerIcon|Colors")
    TArray<FLinearColor> PlayerColors =
    {
        FLinearColor(1.0f, 0.2f, 0.2f, 1.0f),
        FLinearColor(0.2f, 0.4f, 1.0f, 1.0f),
        FLinearColor(0.0f, 0.8f, 0.0f, 1.0f),
        FLinearColor(1.0f, 0.6f, 0.0f, 1.0f)
    };
    
public:
    void SetPlayerIcon(const int index);
    void SetCPUIcon() const;

    // TODO: 
    // void Show();
    // void Hide();
};
