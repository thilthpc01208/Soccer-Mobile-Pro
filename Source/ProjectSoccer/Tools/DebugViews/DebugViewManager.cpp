// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/Tools/DebugViews/DebugViewManager.h"

DEFINE_LOG_CATEGORY(LogDebugViews)

static void RefreshActiveOnWorldBeginPlay()
{
    auto& registry = Internal::DebugViewRegistry::Get();
    registry.RefreshActiveView();
}

namespace Internal
{
	DebugViewRegistry& DebugViewRegistry::Get()
	{
        static DebugViewRegistry Instance;
        return Instance;
	}

    DebugViewRegistry::DebugViewRegistry()
    {
        // Register for when the Gameplay session is started in the Editor.
        if (!GEngine)
            return;

        GEngine->OnWorldAdded().AddLambda([](UWorld* pWorld)
        {
            if (!pWorld)
				return;

            auto& registry = DebugViewRegistry::Get();

            registry.OnWorldBeginHandle = pWorld->OnWorldBeginPlay.AddStatic(&RefreshActiveOnWorldBeginPlay);
        });

        GEngine->OnWorldDestroyed().AddLambda([](UWorld* pWorld)
        {
            if (!pWorld)
                return;

            auto& registry = DebugViewRegistry::Get();

            if (registry.OnWorldBeginHandle.IsValid())
            {
                pWorld->OnWorldBeginPlay.Remove(registry.OnWorldBeginHandle);
                registry.OnWorldBeginHandle.Reset();
            }
        });
    }

	//----------------------------------------------------------------------------------------------------
	//		NOTES:
	//		
	///		@brief : Register a DataTable of DebugViews to the DebugViewRegistry, to be selectable by the user.
    ///		@param pTable : DataTable containing DebugViews. It's RowStruct must be of type FDebugView.
	//----------------------------------------------------------------------------------------------------
	void DebugViewRegistry::RegisterDebugViews(const UDataTable* pTable)
    {
        if (!RegisterViewsFromDataTable(pTable))
            return;

		if (OnDebugViewsUpdatedEvent.IsBound())
			OnDebugViewsUpdatedEvent.Broadcast();
    }

    //----------------------------------------------------------------------------------------------------
	//		NOTES:
	//		
	///		@brief : Remove each DebugView in the DataTable from the DebugViewRegistry.
    ///		@param pTable : DataTable containing DebugViews. It's RowStruct must be of type FDebugView.
	//----------------------------------------------------------------------------------------------------
    void DebugViewRegistry::RemoveAllViewsInTable(const UDataTable* pTable)
    {
        if (!UnregisterViewsFromDataTable(pTable))
            return;

        if (OnDebugViewsUpdatedEvent.IsBound())
			OnDebugViewsUpdatedEvent.Broadcast();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Set the visibility of the ActiveDebugView. If hidden or shown, the DebugViewTargets will
    ///            be toggled accordingly.
    ///		@param bVisible : True to show the active view, false to hide it.
    //----------------------------------------------------------------------------------------------------
    void DebugViewRegistry::SetActiveViewIsVisible(const bool bVisible)
    {
        if (bIsVisible == bVisible)
            return;

        bIsVisible = bVisible;

        // Update the Active Debug View Targets with the new visibility.
        if (ActiveView)
        {
            for (const auto& tag : ActiveDebugViewTags)
            {
                const size_t index = DebugViewTargetMap[tag];
                DebugViewTargets[index].OnDebugViewTargetToggled().Broadcast(bIsVisible);
            }
        }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Set the Active Debug View by name.
    ///		@param name : 
    //----------------------------------------------------------------------------------------------------
    void DebugViewRegistry::SetActiveView(const FName& name)
    {
        // If this is the same as our active view, return.
		if (ActiveView && ActiveView->Name == name)
            return;

        // [NOTE]: This is a linear search of a set, which isn't great, but I don't expect the number of
        // DebugViews to be very large.
        for (const auto* view : DebugViews)
        {
            if (view->Name == name)
            {
                SetActiveView(view);
                return;
            }
        }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : If there is an Active View that is Visible, this will broadcast the OnDebugViewTargetToggled Event for
	///               each DebugViewTarget.
    //----------------------------------------------------------------------------------------------------
    void DebugViewRegistry::RefreshActiveView()
    {
	    if (ActiveView && bIsVisible)
	    {
            for (const auto& tag : ActiveDebugViewTags)
            {
                const size_t index = DebugViewTargetMap[tag];
                DebugViewTargets[index].OnDebugViewTargetToggled().Broadcast(true);
            }
	    }
    }

	//----------------------------------------------------------------------------------------------------
	//		NOTES:
	//		
    ///		@brief : Set a new Active Debug View. This will hide the previous View, as only one is available
    ///             at a time.
	///		@param view : 
	//----------------------------------------------------------------------------------------------------
	void DebugViewRegistry::SetActiveView(const FDebugView* view)
    {
		bIsVisible = true;
        ActiveView = view;
        RefreshActiveTargets(view->TargetTags);
    }

	//----------------------------------------------------------------------------------------------------
	//		NOTES:
	//		
    ///		@brief : Sets the ActiveView to nullptr and disables all active DebugViewTargets.
	//----------------------------------------------------------------------------------------------------
	void DebugViewRegistry::ClearActiveView()
    {
        ActiveView = nullptr;
        RefreshActiveTargets(FGameplayTagContainer());
    }

	//----------------------------------------------------------------------------------------------------
	//		NOTES:
	//		
	///		@brief : Called when the Debug View is changed. This will disable all view targets not in the new collection
    ///             of target tags, and enable all view targets that are in the new collection of target tags.
	///		@param activeTargetTags : 
	//----------------------------------------------------------------------------------------------------
	void DebugViewRegistry::RefreshActiveTargets(const FGameplayTagContainer& activeTargetTags)
    {
        // Disable all views that are not in the new tags.
        for (const auto& tag : ActiveDebugViewTags)
        {
            if (!activeTargetTags.HasTagExact(tag))
            {
                const size_t index = DebugViewTargetMap[tag];
                DebugViewTargets[index].OnDebugViewTargetToggled().Broadcast(false);
            }
        }

        // Set the new active tags.
        ActiveDebugViewTags = activeTargetTags;

        // Enable all views that are in the new tags.
        for (const auto& tag : ActiveDebugViewTags)
        {
            size_t index;
            if (!DebugViewTargetMap.Contains(tag))
            {
                DebugViewTargetMap.Add(tag, DebugViewTargets.Num());
                index = DebugViewTargets.Emplace(tag);
            }

            else
            {
                index = DebugViewTargetMap[tag];
            }

            DebugViewTargets[index].OnDebugViewTargetToggled().Broadcast(true);
        }

    }

	//----------------------------------------------------------------------------------------------------
	//		NOTES:
    //      If I get a better way to automatically unregister for Debug ViewsTargets, I can probably remove this.
    //      - I made this because I had dangling references to object from the last play session that weren't
    //        being removed from the DebugViewTargets' OnDebugViewTargetToggled Event.
	//		
    ///		@brief : Must be called at the end of the game loop to ensure that all DebugViewTargets are properly
    ///             cleaned up.
	//----------------------------------------------------------------------------------------------------
	void DebugViewRegistry::Clear()
    {
        for (auto& debugView : DebugViewTargets)
        {
            debugView.OnDebugViewTargetToggled().Clear();
        }

        DebugViewTargetMap.Empty();
        DebugViewTargets.Empty();
        DebugViews.Empty();
        OnDebugViewsUpdatedEvent.Clear();
        ActiveDebugViewTags.Reset();
        ActiveView = nullptr;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns whether a Debug View is enabled.
    ///		@param targetTag : Gameplay Tag that is mapped to a Debug View.
    ///		@returns : 
    //----------------------------------------------------------------------------------------------------
    bool DebugViewRegistry::IsDebugViewTargetEnabled(const FGameplayTag& targetTag) const
    {
        return ActiveDebugViewTags.HasTagExact(targetTag);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns whether a Debug View is enabled.
    ///		@param tagName : Name of the Gameplay Tag that represents a DebugView.
    ///		@returns : False if not enabled.
    //----------------------------------------------------------------------------------------------------
    bool DebugViewRegistry::IsDebugViewTargetEnabled(const FString& tagName) const
    {
        return IsDebugViewTargetEnabled(FGameplayTag::RequestGameplayTag(FName(*tagName)));
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Try to Register a single DataTable as a DebugView. The DataTable's RowStruct Class must
    ///             be of type FDebugView.
    ///		@param pTable : Table whose views we are trying to register.
    ///		@returns : False if the DataTable is invalid.
    //----------------------------------------------------------------------------------------------------
    bool DebugViewRegistry::RegisterViewsFromDataTable(const UDataTable* pTable)
    {
        // Ensure the DataTable is not null.
        if (!IsValidDataTable(pTable))
        {
            return false;
        }

        TArray<FDebugView*> views;
        pTable->GetAllRows("Grabbing Debug Views Array", views);

        // Add all the Views to the DebugViews set.
        for (const FDebugView* pView : views)
        {
	        check(pView);
            DebugViews.Add(pView);
        }

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Try to unregister all views that are in the provided DataTable. Even if a View is in multiple
    ///              DataTables, it will be removed. If one of the DebugViews is the ActiveView, it will be
    ///              cleared.
    ///		@param pTable : Table whose views we are trying to unregister.
    ///		@returns : False if the DataTable is invalid
    //----------------------------------------------------------------------------------------------------
    bool DebugViewRegistry::UnregisterViewsFromDataTable(const UDataTable* pTable)
    {
	    if (!IsValidDataTable(pTable))
            return false;

        TArray<FDebugView*> views;
        pTable->GetAllRows("Grabbing Debug Views Array", views);

        for (const FDebugView* pView : views)
        {
	        check(pView);

            const auto id = DebugViews.FindId(pView);
            if (!id.IsValidId())
                continue;

            // If this was our active view, clear it.
            if (pView == ActiveView)
                ClearActiveView();

            // Remove all listeners from the DebugViewTarget
            ClearAllTargetsForView(pView);

            // Remove the view from the set.
            DebugViews.Remove(id);
        }

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Clear all DebugViewTarget listeners for a given DebugView. This will *not* remove the
    ///            DebugViewTarget itself.
    //----------------------------------------------------------------------------------------------------
    void DebugViewRegistry::ClearAllTargetsForView(const FDebugView* pView)
    {
        for (const auto& tag : pView->TargetTags)
        {
            if (!DebugViewTargetMap.Contains(tag))
                continue;

            const size_t index = DebugViewTargetMap[tag];
            DebugViewTargets[index].OnDebugViewTargetToggled().Clear();
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether the DataTable is a valid DataTable that holds FDebugViews.
    //----------------------------------------------------------------------------------------------------
    bool DebugViewRegistry::IsValidDataTable(const UDataTable* pTable)
    {
        // Ensure the DataTable is not null.
        if (!pTable)
	    {
            UE_LOG(LogDebugViews, Error, TEXT("DebugViewRegistry::RegisterDebugViews: Invalid DataTable provided."));
            return false;
	    }

        // Ensure the data table is valid.
        const UScriptStruct* pRowStruct = pTable->GetRowStruct();
        if (!pRowStruct)
        {
            UE_LOG(LogDebugViews, Error, TEXT("DebugViewRegistry::RegisterDebugViews: DataTable provided does not have a RowStruct."));
            return false;
        }

        // Ensure this is the right type of DataTable.
        if (pRowStruct != FDebugView::StaticStruct())
        {
            UE_LOG(LogDebugViews, Error, TEXT("DebugViewRegistry::RegisterDebugViews: DataTable provided does not have the correct RowStruct."));
            return false;
        }

        return true;
    }
}