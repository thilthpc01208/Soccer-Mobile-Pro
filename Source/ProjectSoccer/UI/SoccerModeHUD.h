// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SoccerModeHUD.generated.h"

class UUserWidget;

UCLASS()
class PROJECTSOCCER_API ASoccerModeHUD : public AHUD
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Widgets")
    TSubclassOf<UUserWidget> DebugViewWidgetClass;

    UPROPERTY()
    UUserWidget* DebugViewWidget;

public:
	UFUNCTION(BlueprintCallable) void ShowDebugMenu();
    UFUNCTION(BlueprintCallable) void HideDebugMenu();
};