// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectSoccerEditor/Entities/Field/FieldManagerDebugComponent.h"
#include "ProjectSoccer/Entities/Field/FieldManager.h"
#include "ProjectSoccer/Tools/DebugViews/DebugViewManager.h"

DEFINE_DEBUG_VIEW_TARGET_TAG(DV_InfluenceMap, "InfluenceMap");

DEFINE_SOCCER_SHOW_FLAG(FieldManagerDebug, "FieldManager Debug", true);

UFieldManagerDebugComponent::UFieldManagerDebugComponent()
{
    PrimaryComponentTick.TickInterval = 1.f;

    REGISTER_DEBUG_VIEW_TARGET_FUNC(DV_InfluenceMap, this, &UFieldManagerDebugComponent::OnDebugViewTagsChanged);
}

void UFieldManagerDebugComponent::OnRegister()
{
    Super::OnRegister();

    if (AFieldManager* pFieldManager = GetOwner<AFieldManager>())
    {
        UpdateViewHandle = pFieldManager->OnFieldStateUpdated().AddUObject(this, &UFieldManagerDebugComponent::RefreshView);
        UpdatePrimitivesHandle = pFieldManager->OnFieldDimensionsUpdated().AddUObject(this, &UFieldManagerDebugComponent::UpdateFieldDrawPrimitives);
    }
}

void UFieldManagerDebugComponent::OnUnregister()
{
	Super::OnUnregister();

    if (AFieldManager* pFieldManager = GetOwner<AFieldManager>())
    {
        pFieldManager->OnFieldDimensionsUpdated().Remove(UpdatePrimitivesHandle);
    }
}

FDebugRenderSceneProxy* UFieldManagerDebugComponent::CreateDebugSceneProxy()
{
    FFlagBasedRenderSceneProxy* pResult = CreateFlagBasedDebugSceneProxy(ProjectSoccer::Editor::FieldManagerDebugFlagName);

    UWorld* pWorld = GetWorld();
    if (!IsOwningFlagEnabled(pWorld, ProjectSoccer::Editor::FieldManagerDebugFlagName))
    {
	    return pResult;
    }

    if (AFieldManager* pFieldManager = GetOwner<AFieldManager>())
    {
		const auto* pFieldState = pFieldManager->GetFieldState();

    	// Color the InfluenceMap Boxes.
		if (DV_InfluenceMap.IsEnabled())
        {
            const auto& influenceMap = pFieldState->GetInfluenceMap();
        	const auto& influences = influenceMap.GetInfluenceArray();

            for (int i = 0; i < Boxes.Num() - 1; ++i)
            {
                const FColor color = GetColor(influences[i]);
                Boxes[i].Color = color;
            }
        }

        pResult->Boxes = Boxes;
    }

    return pResult;
}

FBoxSphereBounds UFieldManagerDebugComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    FBoxSphereBounds::Builder BoundsBuilder;
	BoundsBuilder += Super::CalcBounds(LocalToWorld);

    // Add the Size of the Field to the Bounds.
    const FVector actorLocation = GetOwner()->GetActorLocation();
    if (AFieldManager* pFieldManager = GetOwner<AFieldManager>())
    {
        const FVector fieldSize = pFieldManager->GetFieldSize();
        const FVector halfFieldSize = fieldSize * 0.5f;

        const FVector min = actorLocation - halfFieldSize;
		const FVector max = actorLocation + halfFieldSize;

        BoundsBuilder += FBox(min, max);
    }

    return BoundsBuilder;
}

void UFieldManagerDebugComponent::RefreshView([[maybe_unused]] const UFieldState& fieldState)
{
    //UpdateFieldDrawPrimitives(fieldSize, influenceTiles);
    MarkRenderStateDirty();
}

void UFieldManagerDebugComponent::UpdateFieldDrawPrimitives(const FVector& fieldSize, const TArray<FBox>& influenceGrid)
{
    if (AFieldManager* pFieldManager = GetOwner<AFieldManager>())
    {
        const FTransform actorTransform = pFieldManager->GetTransform();
        const FVector actorLocation = actorTransform.GetLocation();

        Boxes.Empty();

        // Influence Map Debug View.
        if (DV_InfluenceMap.IsEnabled())
        {
	        Boxes.SetNumZeroed(influenceGrid.Num());

            for (size_t i = 0; i < influenceGrid.Num(); ++i)
            {
                Boxes[i].Box = influenceGrid[i];
                Boxes[i].Transform = actorTransform;
            }
        }

        // [TODO]: Field Zones Debug View.

        // Add a Box for the Field outline.
        Boxes.AddZeroed();
        const FVector halfGridSize = fieldSize * 0.5f;
        const FVector min = actorLocation - halfGridSize;
		const FVector max = actorLocation + halfGridSize;

        // Make an outline of the Field space.
        auto& outlineBox = Boxes.Last();
        outlineBox.Box = FBox(min, max);
        outlineBox.Transform = actorTransform;
        outlineBox.Color = FColor::Yellow;
        outlineBox.DrawTypeOverride = FDebugRenderSceneProxy::EDrawType::WireMesh;
    }
}

FColor UFieldManagerDebugComponent::GetColor(const float influence) const
{
    // Take the positive or negative value and make it normalized.
    const float normalized = (FMath::Clamp<float>(influence , -1.f, 1.f) + 1) * 0.5f;

    // For now, just Blue for positive vs Red for negative.
    const float RScalar = FMath::Clamp<float>((0.5f - normalized) / 0.5f, 0.f, 1.f);
    const float BScalar = FMath::Clamp<float>((normalized - 0.5f) / 0.5f, 0.f, 1.f);
    const uint8 R = static_cast<uint8>(FMath::TruncToInt(255 * RScalar));
    const uint8 B = static_cast<uint8>(FMath::TruncToInt(255 * BScalar));
    return FColor(R, 0, B, 125);
}

void UFieldManagerDebugComponent::OnDebugViewTagsChanged(bool bShow)
{
	// Update the draw primitives:
    if (AFieldManager* pFieldManager = GetOwner<AFieldManager>())
    {
        const auto* fieldState = pFieldManager->GetFieldState();
        const auto fieldSize = fieldState->GetFieldDimensions().GetSize();
		const auto influenceGrid = fieldState->GetInfluenceMap().GetGridTiles();
		UpdateFieldDrawPrimitives(fieldSize, influenceGrid);
    }

    MarkRenderStateDirty();
}

