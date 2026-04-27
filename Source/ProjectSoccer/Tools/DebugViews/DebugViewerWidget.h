// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DebugViewerWidget.generated.h"

class UCheckBox;
class UButton;
class UTextBlock;
class UComboBoxString;

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      This needs to do the following:
//      - Show/Hide Button: I need to know when the ClearActive view is called on the DebugViewManager,
//          so I can update the button text.
//      - Select Debug View from dropdown list.
//          - The list needs to be dynamically populated based on the Debug Views available.
//
///		@brief : Widget that manages enabling and disable Debug Views.
//----------------------------------------------------------------------------------------------------
UCLASS()
class PROJECTSOCCER_API UDebugViewerWidget : public UUserWidget
{
	GENERATED_BODY()

    // [TODO]: This is a hack.
    // I want to be able to read the Player's Input Actions and use that to determine
    // if the Debug Menu should be opened or closed.
    UPROPERTY(EditAnywhere)
    FKey ToggleDebugMenuKey = EKeys::Gamepad_Special_Right;

    UPROPERTY(EditAnywhere, Category = "DebugViewer", meta = (BindWidget))
    UCheckBox* ShowHideCheckBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DebugViewer", meta = (BindWidget, AllowPrivateAccess = "true"))
    UComboBoxString* DebugViewOptions;

    UPROPERTY(EditAnywhere, Category = "DebugViewer")
    FName NoneOptionName = "None";

    bool bWasPausedOnOpen = false;

private:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // HACK:
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    UFUNCTION() void OnShowHideToggleChecked(const bool bChecked);
    UFUNCTION() void OnDebugViewOptionSelected(const FString name, ESelectInfo::Type type);

private:
    void CloseDebugMenu();
};
