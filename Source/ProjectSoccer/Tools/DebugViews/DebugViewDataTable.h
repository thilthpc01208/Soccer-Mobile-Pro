// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DebugViewDataTable.generated.h"

UCLASS(BlueprintType, AutoExpandCategories = "DataTable,ImportOptions", Meta = (LoadBehavior = "LazyOnDemand"))
class PROJECTSOCCER_API UDebugViewDataTable : public UDataTable
{
	GENERATED_BODY()
	
public:
    UDebugViewDataTable();
    
    virtual void PostLoad() override;
    virtual void BeginDestroy() override;
};
