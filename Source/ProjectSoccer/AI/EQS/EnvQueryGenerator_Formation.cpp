// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectSoccer/AI/EQS/EnvQueryGenerator_Formation.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

UEnvQueryGenerator_Formation::UEnvQueryGenerator_Formation(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	//QueryContext = UEnvQueryContext_Querier::StaticClass();
	ItemType = UEnvQueryItemType_Point::StaticClass();
}

void UEnvQueryGenerator_Formation::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
    // Get the array of points from the Context
    TArray<FVector> ContextLocations;
    QueryInstance.PrepareContext(QueryContext, ContextLocations);

    for (const FVector& Location : ContextLocations)
	{
		const FNavLocation NavLoc(Location);
		QueryInstance.AddItemData<UEnvQueryItemType_Point>(NavLoc);
	}
}
