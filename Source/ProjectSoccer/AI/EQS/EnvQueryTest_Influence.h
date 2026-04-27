// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_OnCircle.h"
#include "ProjectSoccer/Entities/Field/InfluenceMap.h"
#include "EnvQueryTest_Influence.generated.h"

class AOutfieldCharacter;
class AInfluenceMap;

UCLASS()
class PROJECTSOCCER_API UEnvQueryTest_Influence : public UEnvQueryTest
{
	GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = Influence)
	TSubclassOf<UEnvQueryContext> InfluenceActorContext;

    UPROPERTY(EditDefaultsOnly, Category = Influence)
    TEnumAsByte<ETargetInfluenceQueryType> TargetInfluenceType;

    UPROPERTY(EditDefaultsOnly, Category = Influence)
    FInfluenceArcQueryParams ArcParams;

public:
    UEnvQueryTest_Influence(const FObjectInitializer& ObjectInitializer);

    //virtual void PostLoad() override;
    virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
    virtual FText GetDescriptionDetails() const override;
};
