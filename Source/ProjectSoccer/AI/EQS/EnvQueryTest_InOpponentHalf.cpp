// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectSoccer/AI/EQS/EnvQueryTest_InOpponentHalf.h"

#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"

UEnvQueryTest_InOpponentHalf::UEnvQueryTest_InOpponentHalf(const FObjectInitializer& ObjectInitializer)
{
    QueryContext = UEnvQueryContext_Querier::StaticClass();
    TestPurpose = EEnvTestPurpose::Filter;
    Cost = EEnvTestCost::Low; // TODO: Make sure this makes sense:
    ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
    BoolValue.DefaultValue = true;
    FilterType = EEnvTestFilterType::Match;
    SetWorkOnFloatValues(false);
}

void UEnvQueryTest_InOpponentHalf::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* pQueryOwner = QueryInstance.Owner.Get();
	if (pQueryOwner == nullptr)
	{
		return;
	}

    const AOutfieldCharacter* pCharacter = Cast<AOutfieldCharacter>(pQueryOwner);
    if (!pCharacter)
        return;

    const ATeamManager* pTeamManager = pCharacter->GetTeamManager();

    BoolValue.BindData(this, QueryInstance.QueryID);
    const bool bWantsInOpponentHalf = BoolValue.GetValue();

    TArray<AActor*> ContextActors;
	if (!QueryInstance.PrepareContext(QueryContext, ContextActors))
	{
		return;
	}

    for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
    {
        const FVector itemLocation = GetItemLocation(QueryInstance, It.GetIndex());
        const bool bInOpponentHalf = !(pTeamManager->LocationInTeamHalf(itemLocation));
        It.SetScore(TestPurpose, FilterType, bInOpponentHalf, bWantsInOpponentHalf);
    }
}

FText UEnvQueryTest_InOpponentHalf::GetDescriptionDetails() const
{
	return DescribeFloatTestParams();
}
