// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/UI/SoccerModeHUD.h"
#include "ProjectSoccer/Tools/DebugViews/DebugViewerWidget.h"

void ASoccerModeHUD::ShowDebugMenu()
{
    if (DebugViewWidgetClass)
    {
        DebugViewWidget = CreateWidget<UUserWidget>(GetWorld(), DebugViewWidgetClass);
        if (DebugViewWidget)
        {
            // Add it to the top most layer.
            DebugViewWidget->AddToViewport(std::numeric_limits<int32>::max());

            // [TODO]: Set any info about the current state of the UI, or key that opened
            // the menu for the widget to use.
            //DebugViewWidget->SetCloseKey() <- Whatever the action key was that called this function.
        }
    }
}

void ASoccerModeHUD::HideDebugMenu()
{
    if (DebugViewWidget)
    {
        DebugViewWidget->RemoveFromParent();
        DebugViewWidget = nullptr;
    }
}