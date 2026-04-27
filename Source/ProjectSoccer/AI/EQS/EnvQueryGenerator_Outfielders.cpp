// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/EQS/EnvQueryGenerator_Outfielders.h"

#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

UEnvQueryGenerator_Outfielders::UEnvQueryGenerator_Outfielders(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	//QueryContext = UEnvQueryContext_Querier::StaticClass();
	ItemType = UEnvQueryItemType_Actor::StaticClass();
}

void UEnvQueryGenerator_Outfielders::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
    TArray<AActor*> ContextActors;
    QueryInstance.PrepareContext(QueryContext, ContextActors);

    for (const AActor* pActor : ContextActors)
	{
		//const FNavLocation NavLoc(pActor->GetActorLocation());
		QueryInstance.AddItemData<UEnvQueryItemType_Actor>(pActor);
	}
}