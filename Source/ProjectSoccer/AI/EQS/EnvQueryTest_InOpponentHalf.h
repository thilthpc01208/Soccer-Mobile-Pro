// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_InOpponentHalf.generated.h"


UCLASS()
class PROJECTSOCCER_API UEnvQueryTest_InOpponentHalf : public UEnvQueryTest
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, Category=InOpponentHalf)
	TSubclassOf<UEnvQueryContext> QueryContext;

public:
    UEnvQueryTest_InOpponentHalf(const FObjectInitializer& ObjectInitializer);

    virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
    virtual FText GetDescriptionDetails() const override;
};
