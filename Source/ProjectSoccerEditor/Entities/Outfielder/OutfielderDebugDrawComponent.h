// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Debug/DebugDrawComponent.h"
#include "ProjectSoccerEditor/Entities/SoccerModeDebugDrawComponent.h"
#include "OutfielderDebugDrawComponent.generated.h"

class AOutfieldCharacter;

UCLASS(ClassGroup=(ProjectSoccer), meta = (BlueprintSpawnableComponent))
class PROJECTSOCCEREDITOR_API UOutfielderDebugDrawComponent : public USoccerModeDebugDrawComponent
{
	GENERATED_BODY()

	UPROPERTY()
    AOutfieldCharacter* Character;

public:
    UOutfielderDebugDrawComponent();
    virtual void BeginPlay() override;
    virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void AddTraceLineToProxy(FFlagBasedRenderSceneProxy* pProxy, const UWorld* pWorld, const FVector start, const AActor* pTarget, const FCollisionQueryParams& params);
    //void ShowPassTargets(bool bShow);
    //void ShowPassingLOS(bool bShow);
    void RefreshView(bool bShow);
};
