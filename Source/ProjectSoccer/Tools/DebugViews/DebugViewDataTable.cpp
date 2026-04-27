// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectSoccer/Tools/DebugViews/DebugViewDataTable.h"

#include "DebugView.h"
#include "DebugViewManager.h"

UDebugViewDataTable::UDebugViewDataTable()
{
    RowStruct = FDebugView::StaticStruct();
}

void UDebugViewDataTable::PostLoad()
{
    Super::PostLoad();

    // Add all the DebugViews that are in this DataTable to the DebugViewRegistry.
    Internal::DebugViewRegistry::Get().RegisterDebugViews(this);
}

void UDebugViewDataTable::BeginDestroy()
{
    Super::BeginDestroy();

    // Remove all the DebugViews that are in this DataTable from the DebugViewRegistry.
    Internal::DebugViewRegistry::Get().RemoveAllViewsInTable(this);
}