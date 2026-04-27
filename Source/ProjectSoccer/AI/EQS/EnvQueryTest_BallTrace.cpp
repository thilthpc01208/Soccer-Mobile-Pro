// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/EQS/EnvQueryTest_BallTrace.h"

#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"

UEnvQueryTest_BallTrace::UEnvQueryTest_BallTrace()
{
	Radius.DefaultValue = 18.f;
    Cost = EEnvTestCost::High;
	ValidItemType = UEnvQueryItemType_Actor::StaticClass();
    SetWorkOnFloatValues(false);
}

void UEnvQueryTest_BallTrace::RunTest(FEnvQueryInstance& QueryInstance) const
{
	const UObject* pOwner = QueryInstance.Owner.Get();
    BoolValue.BindData(pOwner, QueryInstance.QueryID);
    Radius.BindData(pOwner, QueryInstance.QueryID);

    const bool bWantsHit = BoolValue.GetValue();
    const float radius = Radius.GetValue();

    const AOutfieldCharacter* pActor = Cast<AOutfieldCharacter>(pOwner);
    if (!pActor)
        return;

    TArray<FVector> contextLocations;
	if (!QueryInstance.PrepareContext(TraceToContext, contextLocations))
	{
		return;
	}

    FCollisionQueryParams traceParams(SCENE_QUERY_STAT(EnvQueryTrace), false);

    TArray<AActor*> ignoredActors;
	if (QueryInstance.PrepareContext(TraceToContext, ignoredActors))
	{
		traceParams.AddIgnoredActors(ignoredActors);
	}

    ECollisionChannel traceChannel = ECC_Visibility;

    for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
  		const FVector itemLocation = GetItemLocation(QueryInstance, It.GetIndex());
        const AActor* pItemActor = GetItemActor(QueryInstance, It.GetIndex());

		for (int32 ContextIndex = 0; ContextIndex < contextLocations.Num(); ContextIndex++)
		{
			const bool bHit = RunSphereTrace(itemLocation, contextLocations[ContextIndex], pItemActor, QueryInstance.World, traceChannel, traceParams, radius);

			It.SetScore(TestPurpose, FilterType, bHit, bWantsHit);
		}
	}
}

FText UEnvQueryTest_BallTrace::GetDescriptionTitle() const
{
	FString DirectionDesc = 
		FString::Printf(TEXT("to %s"), *UEnvQueryTypes::DescribeContext(TraceToContext).ToString());

	return FText::FromString(FString::Printf(TEXT("%s: %s"),
		*Super::GetDescriptionTitle().ToString(), *DirectionDesc));
}

bool UEnvQueryTest_BallTrace::RunSphereTrace(const FVector& from, const FVector& to, const AActor* pActor, const UWorld* pWorld, ECollisionChannel Channel, const FCollisionQueryParams& params, const float radius) const
{
    FCollisionQueryParams traceParams(params);
    traceParams.AddIgnoredActor(pActor);

    const bool bHit = pWorld->SweepTestByChannel(from, to, FQuat::Identity, Channel,FCollisionShape::MakeSphere(radius), traceParams);
    return bHit;
}

UEnvQueryTest_BallTracePoint::UEnvQueryTest_BallTracePoint()
{
	Radius.DefaultValue = 18.f;
    Cost = EEnvTestCost::High;
	ValidItemType = UEnvQueryItemType_Point::StaticClass();
    SetWorkOnFloatValues(false);
}

void UEnvQueryTest_BallTracePoint::RunTest(FEnvQueryInstance& QueryInstance) const
{
	const UObject* pOwner = QueryInstance.Owner.Get();
    BoolValue.BindData(pOwner, QueryInstance.QueryID);
    Radius.BindData(pOwner, QueryInstance.QueryID);

    const bool bWantsHit = BoolValue.GetValue();
    const float radius = Radius.GetValue();

    const AOutfieldCharacter* pActor = Cast<AOutfieldCharacter>(pOwner);
    if (!pActor)
        return;

    TArray<FVector> contextLocations;
	if (!QueryInstance.PrepareContext(TraceToContext, contextLocations))
	{
		return;
	}

    FCollisionQueryParams traceParams(SCENE_QUERY_STAT(EnvQueryTrace), false);

    TArray<AActor*> ignoredActors;
	if (QueryInstance.PrepareContext(TraceToContext, ignoredActors))
	{
		traceParams.AddIgnoredActors(ignoredActors);
	}

    ECollisionChannel traceChannel = ECC_Visibility;

    for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
  		const FVector itemLocation = GetItemLocation(QueryInstance, It.GetIndex());

		for (int32 ContextIndex = 0; ContextIndex < contextLocations.Num(); ContextIndex++)
		{
			const bool bHit = RunSphereTrace(itemLocation, contextLocations[ContextIndex], QueryInstance.World, traceChannel, traceParams, radius);

			It.SetScore(TestPurpose, FilterType, bHit, bWantsHit);
		}
	}
}

bool UEnvQueryTest_BallTracePoint::RunSphereTrace(const FVector& from, const FVector& to, const UWorld* pWorld, ECollisionChannel Channel, const FCollisionQueryParams& params, const float radius) const
{
    const bool bHit = pWorld->SweepTestByChannel(from, to, FQuat::Identity, Channel,FCollisionShape::MakeSphere(18.f), params);
    return bHit;
}

FText UEnvQueryTest_BallTracePoint::GetDescriptionTitle() const
{
	FString DirectionDesc = 
		FString::Printf(TEXT("to %s"), *UEnvQueryTypes::DescribeContext(TraceToContext).ToString());

	return FText::FromString(FString::Printf(TEXT("%s: %s"),
		*Super::GetDescriptionTitle().ToString(), *DirectionDesc));
}