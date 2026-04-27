// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/EQS/EnvQueryTest_Influence.h"

#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"
#include "ProjectSoccer/Entities/Field/FieldManager.h"

UEnvQueryTest_Influence::UEnvQueryTest_Influence(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TargetInfluenceType()
{
	TestPurpose = EEnvTestPurpose::Score;
	Cost = EEnvTestCost::High; // TODO: Make sure this makes sense:
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}

void UEnvQueryTest_Influence::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* pQueryOwner = QueryInstance.Owner.Get();
	if (pQueryOwner == nullptr)
	{
		return;
	}

    AOutfieldCharacter* pOutfielder = Cast<AOutfieldCharacter>(pQueryOwner);
    if (!pOutfielder)
        return;

    const ATeamManager* pTeamManager = pOutfielder->GetTeamManager();
    const auto& fieldState = pTeamManager->GetFieldState();
    const float teamInfluence = pTeamManager->GetInfluenceValue();

    FloatValueMin.BindData(pQueryOwner, QueryInstance.QueryID);
	const float MinThresholdValue = FloatValueMin.GetValue();

	FloatValueMax.BindData(pQueryOwner, QueryInstance.QueryID);
	const float MaxThresholdValue = FloatValueMax.GetValue();

    TArray<AActor*> ContextActors;
	if (!QueryInstance.PrepareContext(InfluenceActorContext, ContextActors))
	{
		return;
	}

	// For each point that we are evaluating:
    for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
    {
        const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
        for (size_t i = 0; i < ContextActors.Num(); ++i)
        {
            float averageInfluence = 0.f;

            // Calculate the average influence. If this fails, the query fails.
            if (!fieldState.GetInfluenceMap().CalcAverageInfluenceInArc(pOutfielder->GetActorLocation(), pOutfielder->GetActorForwardVector(), ArcParams, averageInfluence))
            {
                It.ForceItemState(EEnvItemStatus::Failed);
            }

	        float score = 0.f;
            switch (TargetInfluenceType)
            {
                case NoInfluence:
                {
                    const float clamped = FMath::Clamp(averageInfluence, -1.f, 1.f);
					score = 1.f - FMath::Abs(clamped);
                    break;
                }

                case Team:
                {
					// If the team influence is -1 and the value is negative, then that results in a higher positive score
                	score = teamInfluence * averageInfluence;
	                break;
                }

                case Opponent:
                {
                	// If the team influence is -1 and the value is positive, then that results in a higher positive score
                	score = -teamInfluence * averageInfluence;
	                break;
                }

                default: break;
            }

			It.SetScore(TestPurpose, FilterType, score, MinThresholdValue, MaxThresholdValue);
        }
    }
}

FText UEnvQueryTest_Influence::GetDescriptionDetails() const
{
	return DescribeFloatTestParams();
}

