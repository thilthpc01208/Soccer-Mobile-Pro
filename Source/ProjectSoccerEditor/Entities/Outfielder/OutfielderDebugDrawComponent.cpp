// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccerEditor/Entities/Outfielder/OutfielderDebugDrawComponent.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjectSoccer/Entities/Field/GoalTrigger.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldAIController.h"
#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"
#include "ProjectSoccer/GameMode/ProjectSoccerGameMode.h"
#include "ProjectSoccerEditor/Entities/SoccerModeDebugDrawComponent.h"

DEFINE_DEBUG_VIEW_TARGET_TAG(DV_OutfielderPassingLOS, "AI.PassingLOS");
DEFINE_DEBUG_VIEW_TARGET_TAG(DV_OutfielderPassRecieve, "Outfielder.PassReceiveLocation");

DEFINE_SOCCER_SHOW_FLAG(OutfielderDebug, "Outfielder Debug", true);

UOutfielderDebugDrawComponent::UOutfielderDebugDrawComponent()
{
    PrimaryComponentTick.TickInterval = 0.075f;

    REGISTER_DEBUG_VIEW_TARGET_FUNC(DV_OutfielderPassingLOS, this, &UOutfielderDebugDrawComponent::RefreshView);
    REGISTER_DEBUG_VIEW_TARGET_FUNC(DV_OutfielderPassRecieve, this, &UOutfielderDebugDrawComponent::RefreshView);
}

void UOutfielderDebugDrawComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AOutfieldCharacter* pCharacter = Cast<AOutfieldCharacter>(GetOwner()))
    {
        Character = pCharacter;
    }
}

FDebugRenderSceneProxy* UOutfielderDebugDrawComponent::CreateDebugSceneProxy()
{
	FFlagBasedRenderSceneProxy* pResult = CreateFlagBasedDebugSceneProxy(ProjectSoccer::Editor::OutfielderDebugFlagName);

	UWorld* pWorld = GetWorld();
	if (!pWorld)
		return pResult;

    if (AOutfieldCharacter* pCharacter = Cast<AOutfieldCharacter>(GetOwner()))
    {
        // If we are not being controlled by an AI controller, return.
        if (!Cast<AOutfieldAIController>(pCharacter->GetController()))
            return pResult;

        const FVector characterLocation = pCharacter->GetActorLocation();
        const UCapsuleComponent* pCapsule = pCharacter->GetComponentByClass<UCapsuleComponent>();
        if (!pCapsule)
            return pResult;

        // If we don't have a TeamManager (Game has started) or we are not in possession:
    	ATeamManager* pTeamManager = pCharacter->GetTeamManager();

        // Pass Receive Location:
        if (DV_OutfielderPassRecieve.IsEnabled())
        {
	        float capsuleOffset = pCapsule->GetScaledCapsuleRadius();
	        capsuleOffset += 1.f;

	        if (!pTeamManager || pTeamManager->GetPossessionState() != ETeamPossessionState::InPossession)
	            return pResult;

	        // Pass Receive Location:
	        const FVector passTarget = pCharacter->GetPassTargetLocation();
	        pResult->Spheres.Emplace(20.f, passTarget, FLinearColor::Yellow);
        }

        // Pass Targets:
        if (DV_OutfielderPassingLOS.IsEnabled())
        {
	        // Ignore ourselves from the trace.
	        FCollisionQueryParams params;
	        params.AddIgnoredActor(pCharacter);

	        // Add the Raycast result to each teammate.
	        const auto& outfielders = pTeamManager->GetOutfielders();
	        for (const auto* pOutfielder : outfielders)
	        {
	            if (pOutfielder == pCharacter)
	                continue;

		        AddTraceLineToProxy(pResult, pWorld, characterLocation, pOutfielder, params);
	        }
        }
    }

    return pResult;
}

FBoxSphereBounds UOutfielderDebugDrawComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds::Builder BoundsBuilder;
    BoundsBuilder += Super::CalcBounds(LocalToWorld);

    // Minimum value around the actor.
    BoundsBuilder += FSphere(GetComponentLocation(), 50.f);

    if (AOutfieldCharacter* pCharacter = Cast<AOutfieldCharacter>(GetOwner()))
    {
	    // If we don't have a TeamManager (Game has started) or we are not in possession:
        ATeamManager* pTeamManager= pCharacter->GetTeamManager();
        if (!pTeamManager)
            return BoundsBuilder;

        FCollisionQueryParams params;
        params.AddIgnoredActor(pCharacter);

        // Add the Play area.
        BoundsBuilder += pTeamManager->GetFieldState().GetFieldDimensions();
    }

    return BoundsBuilder;
}


void UOutfielderDebugDrawComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    MarkRenderStateDirty();
}

void UOutfielderDebugDrawComponent::AddTraceLineToProxy(FFlagBasedRenderSceneProxy* pProxy, const UWorld* pWorld, const FVector start, const AActor* pTarget, const FCollisionQueryParams& params)
{
    const FVector end = pTarget->GetActorLocation();

    // Ignore our specific target:
    FCollisionQueryParams traceParams(params);
    traceParams.AddIgnoredActor(pTarget);

    const bool bHit = pWorld->SweepTestByChannel(start, end, FQuat::Identity, ECC_Visibility ,FCollisionShape::MakeSphere(18.f), traceParams);

    // If we did hit something
	FColor rayColor = bHit? FColor::Red : FColor::Green;
    pProxy->Lines.Emplace(start, end, rayColor, 2.f);
}

//void UOutfielderDebugDrawComponent::ShowPassTargets(bool bShow)
//{
//	if (bShow)
//	{
//        ActiveDrawTargets |= EOutfielderDrawTargets::PassingLOS;
//	}
//
//    else
//    {
//        ActiveDrawTargets &= ~EOutfielderDrawTargets::PassingLOS;
//    }
//
//    MarkRenderStateDirty();
//}
//
//void UOutfielderDebugDrawComponent::ShowPassingLOS(bool bShow)
//{
//    if (bShow)
//    {
//        ActiveDrawTargets |= EOutfielderDrawTargets::PassReceiveLocation;
//    }
//
//    else
//    {
//        ActiveDrawTargets &= ~EOutfielderDrawTargets::PassReceiveLocation;
//    }
//
//    MarkRenderStateDirty();
//}

void UOutfielderDebugDrawComponent::RefreshView([[maybe_unused]] bool bShow)
{
    MarkRenderStateDirty();
}