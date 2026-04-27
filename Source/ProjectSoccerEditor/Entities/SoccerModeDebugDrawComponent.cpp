// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectSoccerEditor/Entities/SoccerModeDebugDrawComponent.h"

//----------------------------------------------------------------------------------------------------
//	Info about the RenderSceneProxy, ShowFlags, and DebugDrawComponent.
//  - https://dev.epicgames.com/community/learning/tutorials/XaE8/unreal-engine-custom-visualization-component-and-show-flags
//		
//----------------------------------------------------------------------------------------------------

FFlagBasedRenderSceneProxy::FFlagBasedRenderSceneProxy(const UPrimitiveComponent* InComponent, const TCHAR* flagName)
    : FDebugRenderSceneProxy(InComponent)
{
    // When drawing a shape should we draw it as a solid mesh or a wireframe or both
	// Solid mesh can be much more expensive if drawing lots of shapes
	// We can also override the draw type for each individual shape
	DrawType = EDrawType::WireMesh;
	// Draw alpha is more or less useless and doesn't do at all what you'd expect it to do
	// It only applies to the SolidMesh draw type
	// The equation for the final alpha of a shape is (Color.A * DrawAlpha) % 255
	// As you can see DrawAlpha is multiplied by the shape's color (assigned when creating the shape) and then is shoved into a uint8 (which is where the % 255 comes from)
	// This is almost never what you'd actually want when adjusting alpha and I strongly suggest leaving this value at 1 and just adjusting alpha in the shape's color itself
	DrawAlpha = 1;
 
	// Set the show flag for this scene proxy
    ViewFlagName = flagName;
    ViewFlagIndex = static_cast<uint32>(FEngineShowFlags::FindIndexByName(*ViewFlagName));
}

FPrimitiveViewRelevance FFlagBasedRenderSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance ViewRelevance;

	ViewRelevance.bDrawRelevance = IsShown(View) && ViewFlagIndex != INDEX_NONE && View->Family->EngineShowFlags.GetSingleFlag(ViewFlagIndex);
	// We need to enable translucency for if we use DrawType SolidMesh
    const bool bIsShown = IsShown(View);
    ViewRelevance.bNormalTranslucency = bIsShown;
	ViewRelevance.bSeparateTranslucency = bIsShown;
	ViewRelevance.bDynamicRelevance = true;
	ViewRelevance.bShadowRelevance = IsShadowCast(View);
	return ViewRelevance;
}

//----------------------------------------------------------------------------------------------------
///		@brief : Set the Default values for the Debug Draw Component.
//----------------------------------------------------------------------------------------------------
USoccerModeDebugDrawComponent::USoccerModeDebugDrawComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    bTickInEditor = true;
	
	SetCastShadow(false);
	SetHiddenInGame(false);
	bVisibleInReflectionCaptures = false;
	bVisibleInRayTracing = false;
	bVisibleInRealTimeSkyCaptures = false;
	// Exclude this component from non-editor builds
	bIsEditorOnly = true;
	
#if WITH_EDITORONLY_DATA
	SetIsVisualizationComponent(true);
#endif;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Create the FlagBasedRenderSceneProxy for the Debug Draw Component.
///		@param flagName : Name of the Flag that determines whether this component is visible.
///		@returns : New RenderSceneProxy.
//----------------------------------------------------------------------------------------------------
FFlagBasedRenderSceneProxy* USoccerModeDebugDrawComponent::CreateFlagBasedDebugSceneProxy(const TCHAR* flagName) const
{
    return new FFlagBasedRenderSceneProxy(this, flagName);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Returns true if the EngineShowFlag with the given name is enabled.
///		@param pWorld : World that we are checking the flag for.
///		@param flagName : Name of the Flag.
///		@returns : 
//----------------------------------------------------------------------------------------------------
bool USoccerModeDebugDrawComponent::IsOwningFlagEnabled(UWorld* pWorld, const TCHAR* flagName) const
{
    check(pWorld);
    const uint32 index = pWorld->GetGameViewport()->EngineShowFlags.FindIndexByName(flagName);
	FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(pWorld);

    bool bShowFlagSet = false;

#if WITH_EDITOR
	if (GEditor && WorldContext && WorldContext->WorldType != EWorldType::Game)
	{
		bShowFlagSet = WorldContext->GameViewport != nullptr && WorldContext->GameViewport->EngineShowFlags.GetSingleFlag(index);
		if (!bShowFlagSet)
		{
			// We have to check all viewports because we can't distinguish between SIE and PIE at this point
			for(const FEditorViewportClient* CurrentViewport : GEditor->GetAllViewportClients())
			{
				if(CurrentViewport && CurrentViewport->EngineShowFlags.GetSingleFlag(index))
				{
					bShowFlagSet = true;
					break;
				}
			}
		}
	}

	else
#endif
	{
		bShowFlagSet = WorldContext && WorldContext->GameViewport && WorldContext->GameViewport->EngineShowFlags.GetSingleFlag(index);
	}
 
	return bShowFlagSet;
}