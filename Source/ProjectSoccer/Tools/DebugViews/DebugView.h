// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "DebugView.generated.h"

namespace Internal
{
	//----------------------------------------------------------------------------------------------------
	///		@brief : Defines a DebugView Target Tag, with an optional Dev description. This should be only be
    ///              created by way of the DEFINE_DEBUGVIEW_TARGET_TAG macro in a cpp file. It is intended to
    ///              convenience class for checking the state of a Debug View Target Tag, and registering to
    ///              be notified when the state changes.
	//----------------------------------------------------------------------------------------------------
	struct PROJECTSOCCER_API FNativeDebugViewTargetTag
	{
	private:
		FNativeGameplayTag Tag;

	public:
	    FNativeDebugViewTargetTag(FName TagName, const FString& TagDevComment);

	    FGameplayTag GetTag() const;
	    bool IsEnabled() const;
	};
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Declare a DebugView Target Tag. This declares a new GameplayTag to be defined in a cpp file.
///              The macro to define the tag is DEFINE_DEBUGVIEW_TARGET_TAG(Name).
///		@param VarName : Name of the Debug Target Tag. This is the Name of the Variable that will be used in
///                   when defining it in a cpp file.
//----------------------------------------------------------------------------------------------------
#define DECLARE_DEBUG_VIEW_TARGET_TAG(VarName) extern Internal::FNativeDebugViewTargetTag VarName

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Defines a DebugView Target Tag. This defines the GameplayTag that was optionally declared
///              in a header file. This will create a new GameplayTag with the fullname of "DebugView.Name".
///              This will place the Tags as children of the "DebugView" parent tag.
///              - If you want to make a hierarchy of tags, you can use the "." character to separate them.
///                Example: <code>DEFINE_DEBUGVIEW_TARGET_TAG(DV_Pathfinding, "AI.Pathfinding");</code>
///                - This will make the Tag with the name: "DebugView.AI.Pathfinding"
///		@param VarName : Name of global variable used to reference the Tag in code.
///     @param TagName : Name of the Tag. This will be the name of the Tag in the editor, prefixed with "DebugView."
//----------------------------------------------------------------------------------------------------
#define DEFINE_DEBUG_VIEW_TARGET_TAG(VarName, TagName) Internal::FNativeDebugViewTargetTag VarName("DebugView." TagName, "")

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      [TODO, Maybe]: Home vs Away Info here? I want to be able to differentiate between Home, Away, and Both.
//		
///		@brief : A Debug View is a collection of FGameplayTags that are used to enable
///              debug visuals for entities associated with those tags.
///             - For example, a Debug View for "AI" which could turn on the "AI.Pathfinding" & "AI.Behavior"
///               tags. Any debug visual component or widget associated with those tags would then be
///               enabled.
//----------------------------------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct PROJECTSOCCER_API FDebugView : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FName Name;

    UPROPERTY(EditAnywhere, meta = (Categories = "DebugView"))
	FGameplayTagContainer TargetTags;

    //virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override;
};