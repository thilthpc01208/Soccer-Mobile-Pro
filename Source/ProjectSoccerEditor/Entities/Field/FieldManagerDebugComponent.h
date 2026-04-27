// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Debug/DebugDrawComponent.h"
#include "ProjectSoccer/Tools/DebugViews/DebugView.h"
#include "ProjectSoccerEditor/Entities/SoccerModeDebugDrawComponent.h"
//#include "GameplayTagContainer.h"
#include "FieldManagerDebugComponent.generated.h"

class UFieldState;

DECLARE_DEBUG_VIEW_TARGET_TAG(DV_InfluenceMap);

UCLASS(ClassGroup=(ProjectSoccer), meta = (BlueprintSpawnableComponent))
class PROJECTSOCCEREDITOR_API UFieldManagerDebugComponent : public USoccerModeDebugDrawComponent
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TObjectPtr<UMaterial> MeshMaterial;

    TArray<FDebugRenderSceneProxy::FDebugBox> Boxes;
    FDelegateHandle UpdateViewHandle;
    FDelegateHandle UpdatePrimitivesHandle;
    //FColor HomeColor; // TODO: It'd be nice to have the colors based on the team colors.
    //FColor AwayColor; // TODO: It'd be nice to have the colors based on the team colors.

public:
    UFieldManagerDebugComponent();

    virtual void OnRegister() override;
    virtual void OnUnregister() override;
    virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
    virtual bool ShouldRecreateProxyOnUpdateTransform() const override { return true; }

private:
    void RefreshView(const UFieldState& fieldState);
    void UpdateFieldDrawPrimitives(const FVector& fieldSize, const TArray<FBox>& influenceGrid);
    FColor GetColor(const float influence) const;

    void OnDebugViewTagsChanged(bool bShow);
    //void AddInfluenceMapPrimitives(const UFieldState* fieldState);
    //void ShowMap(bool bShow);
};
