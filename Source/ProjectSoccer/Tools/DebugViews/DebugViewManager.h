// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DebugView.h"
#include "GameplayTagContainer.h"

class UDebugViewDataTable;

template <typename Type>
concept UObjectType = requires(Type t, const bool bEnabled)
{
    std::is_base_of_v<UObject, Type>;
    //{ t.ToggleDebugView(bEnabled) } -> void;
};

PROJECTSOCCER_API DECLARE_LOG_CATEGORY_EXTERN(LogDebugViews, Log, All);

namespace Internal
{
	DECLARE_EVENT_OneParam(DebugViewTarget, FOnToggleDebugView, bool /*bEnabled*/);
    DECLARE_EVENT(DebugViewRegistry, FOnDebugViewsUpdated);

	//----------------------------------------------------------------------------------------------------
	//		NOTES:
    //      [TODO]: Move to DebugView.h
	//		
	///		@brief : Struct with a Tag identifier and a Multicast Delegate Event that is broadcast when
	///            this DebugViewTarget is enabled or disabled, which entities can subscribe to.
    ///            - You don't directly interact with this class; Registering to a DebugViewTarget
	///               happens through the DebugViewRegistry.
	//----------------------------------------------------------------------------------------------------
	class PROJECTSOCCER_API DebugViewTarget
	{
        FGameplayTag Tag;
        FOnToggleDebugView OnDebugViewTargetToggledEvent;

	public:
        DebugViewTarget(const FGameplayTag& tag) : Tag(tag) {}

        const FGameplayTag& GetTag() const { return Tag; }
        FOnToggleDebugView& OnDebugViewTargetToggled() { return OnDebugViewTargetToggledEvent; }
	};

	//----------------------------------------------------------------------------------------------------
	//		NOTES:
    //      - When using the editor, the static instance exists for the entire time the editor is open.
    //        So the Views will persist between individual play sessions when using the editor.
	//		
    ///		@brief : Internal class with a static interface for registering DebugViews.
	//----------------------------------------------------------------------------------------------------
	class PROJECTSOCCER_API DebugViewRegistry
    {
        TMap<FGameplayTag, size_t> DebugViewTargetMap; // Maps a Tag to an index in the DebugViewTargets array.
        TSet<const FDebugView*> DebugViews;			   // Contains all registered DebugViews.  
        TArray<DebugViewTarget> DebugViewTargets;      // Contains all registered DebugViewTargets.
        FGameplayTagContainer ActiveDebugViewTags;     // DebugTargetTags that are currently active
        FOnDebugViewsUpdated OnDebugViewsUpdatedEvent; // Event that is broadcast when the DebugViews Set is updated.
        const FDebugView* ActiveView = nullptr;        // The current active view.
        bool bIsVisible = false;					   // Whether the ActiveView is visible.

        FDelegateHandle OnWorldBeginHandle;

        DebugViewRegistry();

    public:
        static DebugViewRegistry& Get();

        void RegisterDebugViews(const UDataTable* pTable);
        void RemoveAllViewsInTable(const UDataTable* pTable);

        template <UObjectType TargetType>
        void RegisterTargetInstance(const FGameplayTag& tag, TargetType* pObject, void(TargetType::* func)(const bool));

        void RefreshActiveView();
        void SetActiveView(const FDebugView* pView);
        void SetActiveView(const FName& name);
        void SetActiveViewIsVisible(const bool bVisible);
        void ClearActiveView();
        void Clear();

        bool IsDebugViewTargetEnabled(const FGameplayTag& targetTag) const;
        bool IsDebugViewTargetEnabled(const FString& tagName) const;

        bool IsActiveViewVisible() const                                { return bIsVisible; }
        const FDebugView* GetActiveView() const                         { return ActiveView; }
        const TSet<const FDebugView*>& GetRegisteredDebugViews() const  { return DebugViews; }
        FOnDebugViewsUpdated& OnRegistryUpdated()                       { return OnDebugViewsUpdatedEvent; }

	private:
        void RefreshActiveTargets(const FGameplayTagContainer& activeTargetTags);
        bool RegisterViewsFromDataTable(const UDataTable* pTable);
        bool UnregisterViewsFromDataTable(const UDataTable* pTable);
        void ClearAllTargetsForView(const FDebugView* pView);
        static bool IsValidDataTable(const UDataTable* pTable);
    };
}

#if !UE_BUILD_SHIPPING
#define REGISTER_DEBUG_VIEW_TARGET_FUNC(NativeTag, this, FuncName) Internal::DebugViewRegistry::Get().RegisterTargetInstance(NativeTag.GetTag(), this, FuncName)
#else
#define REGISTER_DEBUG_VIEW_TARGET_FUNC(NativeTag, this, FuncName) void(0)
#endif

namespace Internal
{
	//----------------------------------------------------------------------------------------------------
	//		NOTES:
	//		[TODO]: I want to get rid of the FString tagName parameter in favor of a data-driven object.
    //         - This object should be on the class so that it can set its Target Tags in the editor.
	//         - This declaration should be a macro, just so it can be compiled away in shipping builds.
    //
    ///		@brief : Register a UObject's function to be called when a DebugViewTarget is toggled.
    ///		@tparam TargetType : Type of UObject that is registering.
    ///		@param tag : Name of the DebugViewTargetTag to register to.
	///		@param pObject : Object that is being registered.
    ///		@param func : Member function to call when the DebugViewTarget is toggled. It takes in a bool, which
    ///            represents whether the DebugViewTarget is enabled or disabled.
	//----------------------------------------------------------------------------------------------------
	template <UObjectType TargetType>
    void DebugViewRegistry::RegisterTargetInstance(const FGameplayTag& tag, TargetType* pObject, void(TargetType::* func)(const bool))
    {
        //[[maybe_unused]] FGameplayTag tag = FGameplayTag::RequestGameplayTag(FName(*tagName));
        
        int32 index;

        // If we don't have this Tag registered yet, create a new DebugViewTarget for it.
        if (!DebugViewTargetMap.Contains(tag))
        {
			UE_LOG(LogDebugViews, Log, TEXT("Registering Target to new DebugView Tag: %s"), *tag.GetTagName().ToString());

            index = DebugViewTargets.Emplace(tag);
            DebugViewTargetMap.Emplace(tag, index);
        }

        // Else, grab the tag that is already registered.
        else
        {
			UE_LOG(LogDebugViews, Log, TEXT("Adding Target to existing DebugView Tag: %s"), *tag.GetTagName().ToString());

            index = DebugViewTargetMap[tag];
            check(index < DebugViewTargets.Num());
        }

        // Add the function to the DebugViewTarget's event.
		DebugViewTargets[index].OnDebugViewTargetToggled().AddUObject(pObject, func);

        // If the DebugView that this Target is associated with is active, enable it immediately.
        for (const auto activeTag : ActiveDebugViewTags)
        {
	        if (activeTag == tag)
	        {
                (pObject->*func)(true);
	        }
        }
    }
}