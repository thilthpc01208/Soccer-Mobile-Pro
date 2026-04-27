// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectSoccerEditor/Public/Tools/DebugViewDataTableFactory.h"
#include "ProjectSoccer/Tools/DebugViews/DebugViewDataTable.h"
#include "ProjectSoccer/Tools/DebugViews/DebugView.h"

UDebugViewDataTableFactory::UDebugViewDataTableFactory()
{
	SupportedClass = UDebugViewDataTable::StaticClass();
}

bool UDebugViewDataTableFactory::ConfigureProperties()
{
	Struct = FDebugView::StaticStruct();
	return true;
}

UObject* UDebugViewDataTableFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* FeedbackContext)
{
	return NewObject<UDebugViewDataTable>(InParent, Class, Name, Flags, Context);
}