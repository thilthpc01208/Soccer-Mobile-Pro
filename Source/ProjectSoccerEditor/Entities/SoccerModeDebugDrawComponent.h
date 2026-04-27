// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Debug/DebugDrawComponent.h"
#include "SoccerModeDebugDrawComponent.generated.h"

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Define a global TCustomShowFlag for the Soccer project. The declaration will be in
///              the ProjectSoccer::Editor namespace. The final name is Show##flagName##Flag. This
///              should be used in the .cpp file.
///		@param flagName : Name of the flag. Ex: OutfielderDebug
///		@param displayName : String to display in the Editor. Ex: "Outfielder Debug"
///		@param defaultEnabled : Whether the flag is enabled by default. True or false.
//----------------------------------------------------------------------------------------------------
#define DEFINE_SOCCER_SHOW_FLAG(flagName, displayName, defaultEnabled)                                                                      \
namespace ProjectSoccer                                                                                                     \
{                                                                                                                           \
	namespace Editor                                                                                                        \
	{                                                                                                                       \
        constexpr auto flagName##FlagName = TEXT(#flagName);														        \
        TCustomShowFlag<> Show##flagName##Flag(flagName##FlagName, defaultEnabled, SFG_Developer, FText::FromString(displayName));    \
	}                                                                                                                       \
}                                                                                                                           

class PROJECTSOCCEREDITOR_API FFlagBasedRenderSceneProxy final : public FDebugRenderSceneProxy
{
public:
    FFlagBasedRenderSceneProxy(const UPrimitiveComponent* InComponent, const TCHAR* flagName);

protected:
    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

    // You only need to override this if you want to perform custom drawing that isn't supported by FDebugRenderSceneProxy's preset shapes
	//virtual void GetDynamicMeshElementsForView(const FSceneView* View, const int32 ViewIndex, const FSceneViewFamily& ViewFamily, const uint32 VisibilityMap, FMeshElementCollector& Collector, FMaterialCache& DefaultMaterialCache, FMaterialCache& SolidMeshMaterialCache) const override;
};

UCLASS()
class PROJECTSOCCEREDITOR_API USoccerModeDebugDrawComponent : public UDebugDrawComponent
{
	GENERATED_BODY()


protected:
    USoccerModeDebugDrawComponent();

    FFlagBasedRenderSceneProxy* CreateFlagBasedDebugSceneProxy(const TCHAR* flagName) const;
    bool IsOwningFlagEnabled(UWorld* pWorld, const TCHAR* flagName) const;
};
