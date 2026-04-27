// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccerEditor/Entities/Team/TeamManagerDebugDrawComponent.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"
#include "ProjectSoccer/Tools/DebugViews/DebugViewManager.h"

DEFINE_DEBUG_VIEW_TARGET_TAG(DV_CurrentFormation, "Team.CurrentFormation");

DEFINE_SOCCER_SHOW_FLAG(TeamManagerDebug, "TeamManager Debug", true);

UTeamManagerDebugDrawComponent::UTeamManagerDebugDrawComponent()
{
    PrimaryComponentTick.TickInterval = 1.f;
    REGISTER_DEBUG_VIEW_TARGET_FUNC(DV_CurrentFormation, this, &UTeamManagerDebugDrawComponent::ShowCurrentFormation);
}

void UTeamManagerDebugDrawComponent::OnRegister()
{
	Super::OnRegister();

    if (ATeamManager* pTeamManager = GetOwner<ATeamManager>())
    {
        if (pTeamManager)
			UpdateRenderHandle = pTeamManager->OnFormationPositionAssigned().AddUObject(this, &UTeamManagerDebugDrawComponent::Refresh);
    }
}

void UTeamManagerDebugDrawComponent::OnUnregister()
{
	Super::OnUnregister();

    if (ATeamManager* pTeamManager = GetOwner<ATeamManager>())
    {
		if (pTeamManager != nullptr)
			pTeamManager->OnFormationPositionAssigned().Remove(UpdateRenderHandle);
    }
}

FDebugRenderSceneProxy* UTeamManagerDebugDrawComponent::CreateDebugSceneProxy()
{
    FFlagBasedRenderSceneProxy* pResult = CreateFlagBasedDebugSceneProxy(ProjectSoccer::Editor::TeamManagerDebugFlagName);

    if (!DV_CurrentFormation.IsEnabled())
        return pResult;

    if (ATeamManager* pTeamManager = Cast<ATeamManager>(GetOwner()))
	{
        const auto* pCurrentFormation = pTeamManager->GetCurrentFormation();
        if (pCurrentFormation != nullptr)
        {
            const size_t count = pCurrentFormation->GetPointCount();

	        for (size_t i = 0; i < count; ++i)
	        {
				pResult->Spheres.Emplace(FormationPositionSphereRadius, pCurrentFormation->GetLocationOfPoint(i), pCurrentFormation->GetDebugColor(i));
	        }
        }
	}

	return pResult;
}

FBoxSphereBounds UTeamManagerDebugDrawComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds::Builder BoundsBuilder;
	BoundsBuilder += Super::CalcBounds(LocalToWorld);
	BoundsBuilder += FSphere(GetComponentLocation(), 50.f);
	if (ATeamManager* pTeamManager = Cast<ATeamManager>(GetOwner()))
	{
		// Expand our bounds to include all our current formation's points.
        const auto* pCurrentFormation = pTeamManager->GetCurrentFormation();
        if (pCurrentFormation != nullptr)
        {
	        const auto& points = pTeamManager->GetCurrentFormation()->GetPointsConst();

	        for (size_t i = 0; i < points.Num(); ++i)
	        {
				BoundsBuilder += points[i];
	        }
        }
	}
 
	return BoundsBuilder;
}

void UTeamManagerDebugDrawComponent::Refresh()
{
	MarkRenderStateDirty();
}

void UTeamManagerDebugDrawComponent::ShowCurrentFormation(const bool bShow)
{
    MarkRenderStateDirty();
}

