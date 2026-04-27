// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectSoccer/Tools/DebugViews/DebugView.h"
#include "DebugViewManager.h"

namespace Internal
{
	FNativeDebugViewTargetTag::FNativeDebugViewTargetTag(FName TagName, const FString& TagDevComment)
        : Tag(UE_PLUGIN_NAME, UE_MODULE_NAME, TagName, TagDevComment, ENativeGameplayTagToken::PRIVATE_USE_MACRO_INSTEAD)
	{
		//
	}

	//----------------------------------------------------------------------------------------------------
    ///		@brief : Get the internal Gameplay Tag of the DebugView Target Tag.
	//----------------------------------------------------------------------------------------------------
	FGameplayTag FNativeDebugViewTargetTag::GetTag() const
    {
	    return Tag;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether the Debug View Target Tag is enabled by the current DebugView.
    //----------------------------------------------------------------------------------------------------
    bool FNativeDebugViewTargetTag::IsEnabled() const
    {
        const auto& debugViewRegistry = DebugViewRegistry::Get();
        return debugViewRegistry.IsActiveViewVisible() && debugViewRegistry.IsDebugViewTargetEnabled(Tag);
    }
}
