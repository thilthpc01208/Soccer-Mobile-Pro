// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccerEditor/Public/Tools/DebugViewDataTableAssetTypeActions.h"

#include "IDataTableEditor.h"
#include "DataTableEditorModule.h"
#include "ProjectSoccer/Tools/DebugViews/DebugViewDataTable.h"

UClass* FDebugViewDataTableAssetTypeActions::GetSupportedClass() const
{
	return UDebugViewDataTable::StaticClass();
}

FText FDebugViewDataTableAssetTypeActions::GetName() const
{
	return INVTEXT("DebugViewDataTable");
}

FColor FDebugViewDataTableAssetTypeActions::GetTypeColor() const
{
	return FColor::Green;
}

uint32 FDebugViewDataTableAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

void FDebugViewDataTableAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	//MakeShared<FDataTableEditor>()->InitDataTableEditor(InObjects);
}
