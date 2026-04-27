// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_BallTrace.generated.h"

UCLASS()
class PROJECTSOCCER_API UEnvQueryTest_BallTrace : public UEnvQueryTest
{
	GENERATED_BODY()

    /** The Context we are tracing to.*/
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	TSubclassOf<UEnvQueryContext> TraceToContext;

    UPROPERTY(EditDefaultsOnly, Category=Trace)
	FAIDataProviderFloatValue Radius;

    //UPROPERTY(EditDefaultsOnly, Category=Trace)
	//FAIDataProviderFloatValue Radius;

public:
    UEnvQueryTest_BallTrace();

    virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
    virtual FText GetDescriptionTitle() const override;
private:
    bool RunSphereTrace(const FVector& from, const FVector& to, const AActor* pActor, const UWorld* pWorld, ECollisionChannel Channel, const FCollisionQueryParams& params, const float radius) const;
};

UCLASS()
class PROJECTSOCCER_API UEnvQueryTest_BallTracePoint : public UEnvQueryTest
{
	GENERATED_BODY()

    /** The Context we are tracing to.*/
	UPROPERTY(EditDefaultsOnly, Category=Trace)
	TSubclassOf<UEnvQueryContext> TraceToContext;

    UPROPERTY(EditDefaultsOnly, Category=Trace)
	FAIDataProviderFloatValue Radius;

public:
    UEnvQueryTest_BallTracePoint();

    virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
    virtual FText GetDescriptionTitle() const override;
private:
    bool RunSphereTrace(const FVector& from, const FVector& to, const UWorld* pWorld, ECollisionChannel Channel, const FCollisionQueryParams& params, const float radius) const;
};
