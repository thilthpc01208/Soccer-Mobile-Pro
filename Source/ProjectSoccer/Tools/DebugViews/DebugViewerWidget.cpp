// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/Tools/DebugViews/DebugViewerWidget.h"
#include "DebugViewManager.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"

void UDebugViewerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetIsFocusable(true);
    //DebugViewOptions->OnGenerateItemWidget.BindDynamic(this, &UDebugViewerWidget::OnGenerateItemWidget);
    //DebugViewOptions->OnGenerateContentWidget.BindDynamic(this, &UDebugViewerWidget::OnGenerateContentWidget);
    DebugViewOptions->AddOption(NoneOptionName.ToString());

    // Populate the DebugViewOptions ComboBox with the available Debug Views.
    const auto& debugViewsSet = Internal::DebugViewRegistry::Get().GetRegisteredDebugViews();
    for (auto* pDebugView : debugViewsSet)
    {
        DebugViewOptions->AddOption(pDebugView->Name.ToString());
    }

    // If there is a current active view, set it as the selected option.
    const auto* pActiveView = Internal::DebugViewRegistry::Get().GetActiveView();
    if (pActiveView)
        DebugViewOptions->SetSelectedOption(pActiveView->Name.ToString());
    else
        DebugViewOptions->SetSelectedOption(NoneOptionName.ToString());

    // Register for changes in the DebugViewOptions ComboBox.
    DebugViewOptions->OnSelectionChanged.AddDynamic(this, &UDebugViewerWidget::OnDebugViewOptionSelected);

    // Set our initial checked state based on the DebugViewRegistry.
    const bool bIsVisible = Internal::DebugViewRegistry::Get().IsActiveViewVisible();
    ShowHideCheckBox->SetCheckedState(bIsVisible ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);

    // Register for changes to the Active Debug View.
    ShowHideCheckBox->OnCheckStateChanged.AddDynamic(this, &UDebugViewerWidget::OnShowHideToggleChecked);

    // [TODO]: Should this be here or in the HUD class that creates this Widget?
    // Pause the Game:
    APlayerController* pPlayerController = GetOwningPlayer();
    if (pPlayerController)
    {
        pPlayerController->SetInputMode(FInputModeUIOnly());
        pPlayerController->SetShowMouseCursor(true);

        bWasPausedOnOpen = pPlayerController->IsPaused();
        pPlayerController->SetPause(true);

        SetFocus();
    }
}

void UDebugViewerWidget::NativeDestruct()
{
    Super::NativeDestruct();

    DebugViewOptions->OnSelectionChanged.RemoveDynamic(this, &UDebugViewerWidget::OnDebugViewOptionSelected);
    ShowHideCheckBox->OnCheckStateChanged.RemoveDynamic(this, &UDebugViewerWidget::OnShowHideToggleChecked);
    //Internal::DebugViewRegistry::Get().OnRegistryUpdated().(this, &UDebugViewerWidget::OnDebugViewsListChanged);
}

// [HACK] : This is temp solution...
// - I need to be able to read the Player's Input Actions and use that to determine
//  if the Debug Menu should be opened or closed.
FReply UDebugViewerWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == ToggleDebugMenuKey)
    {
        CloseDebugMenu();
        return FReply::Handled();
    }

    return FReply::Unhandled();
}

void UDebugViewerWidget::OnShowHideToggleChecked(const bool bChecked)
{
	Internal::DebugViewRegistry::Get().SetActiveViewIsVisible(bChecked);
}

void UDebugViewerWidget::OnDebugViewOptionSelected(const FString name, ESelectInfo::Type type)
{
    if (name == NoneOptionName)
    {
        Internal::DebugViewRegistry::Get().ClearActiveView();
        ShowHideCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
        ShowHideCheckBox->SetIsEnabled(false);
        return;
    }

    Internal::DebugViewRegistry::Get().SetActiveView(FName(name));
	ShowHideCheckBox->SetIsEnabled(true);
    ShowHideCheckBox->SetCheckedState(ECheckBoxState::Checked);
}

void UDebugViewerWidget::CloseDebugMenu()
{
    // Hide the Debug Menu.
    if (APlayerController* pPlayerController = GetOwningPlayer())
    {
        // [TODO]: I should return to whatever input mode was active before opening the Debug Menu.
        //         - I don't know how to grab this information yet.
        //         - Same for the mouse cursor.
        pPlayerController->SetInputMode(FInputModeGameOnly());
        pPlayerController->SetShowMouseCursor(false);

        if (!bWasPausedOnOpen)
        	pPlayerController->SetPause(false);
        
        RemoveFromParent();
    }
}