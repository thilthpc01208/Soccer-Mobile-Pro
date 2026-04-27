// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "EnvQueryGenerator_Outfielders.generated.h"

UCLASS()
class PROJECTSOCCER_API UEnvQueryGenerator_Outfielders : public UEnvQueryGenerator
{
	GENERATED_BODY()
	
protected:
	/** context */
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	TSubclassOf<UEnvQueryContext> QueryContext;

public:
    UEnvQueryGenerator_Outfielders(const FObjectInitializer& ObjectInitializer);

	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;
};
