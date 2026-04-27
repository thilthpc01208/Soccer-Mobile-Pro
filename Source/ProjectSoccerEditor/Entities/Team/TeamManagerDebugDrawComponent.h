// Fill out your copyright notice in the Description page of Project Settings.
// Tutorial Link: https://dev.epicgames.com/community/learning/tutorials/XaE8/unreal-engine-custom-visualization-component-and-show-flags

#pragma once

#include "CoreMinimal.h"
#include "Debug/DebugDrawComponent.h"
#include "../SoccerModeDebugDrawComponent.h"
#include "TeamManagerDebugDrawComponent.generated.h"

UCLASS(ClassGroup=(ProjectSoccer), meta = (BlueprintSpawnableComponent))
class PROJECTSOCCEREDITOR_API UTeamManagerDebugDrawComponent : public USoccerModeDebugDrawComponent
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    float FormationPositionSphereRadius = 50.f;

    FDelegateHandle UpdateRenderHandle;

public:
    UTeamManagerDebugDrawComponent();

    virtual void OnRegister() override;
    virtual void OnUnregister() override;
    virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
    virtual bool ShouldRecreateProxyOnUpdateTransform() const override { return true; }

private:
    void Refresh();
    void ShowCurrentFormation(const bool bShow);
};
