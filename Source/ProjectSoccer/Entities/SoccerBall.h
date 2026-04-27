// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/SphereComponent.h"
#include "Field/ArcMovementComponent.h"
#include "GameFramework/Actor.h"
#include "ProjectSoccer/GameMode/MatchEntityInterface.h"
#include "SoccerBall.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSoccerBall, Log, All);

class AOutfieldCharacter;

struct FKickParams
{
	enum class EType
    {
        Null,
        Pass,
        Shot,
        // Clear,
        // Lob,
    };

    FVector TargetLocation = FVector::ZeroVector;
    FVector TargetVelocity = FVector::ZeroVector;
    class UCurveFloat* VelocityCurve = nullptr;
    float ArcCurve = 0.f;  // TODO: 
    float MaxArcHeightOffset = 1.f;
    float Speed = 1.f;
    EType Type = EType::Null;
};


UCLASS()
class PROJECTSOCCER_API ASoccerBall : public AActor, public IMatchEntityInterface
{
	GENERATED_BODY()

public:
	//----------------------------------------------------------------------------------------------------
    ///		@brief : Info about the last contact with the ball.
	//----------------------------------------------------------------------------------------------------
	struct FContactInfo
    {
        enum class EType
        {
            Kick,
	        Shot,
            // Block,
        };

        TObjectPtr<AOutfieldCharacter> Outfielder = nullptr;
        FVector Location = FVector::Zero();
        EType Type = EType::Shot;
    };

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
    class UStaticMeshComponent* Mesh;

     UPROPERTY(VisibleAnywhere, Category = Default, meta = (AllowPrivateAccess = "true"))
    class UArcMovementComponent* ArcMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Default, meta = (AllowPrivateAccess = "true"))
    class USphereComponent* SphereCollider;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Default, meta = (AllowPrivateAccess = "true"))
    float BaseSpeed;

    UPROPERTY()
    TObjectPtr<AOutfieldCharacter> PlayerInPossession;

    UPROPERTY()
    TObjectPtr<AProjectSoccerGameMode> GameMode;

    FKickParams LastKickParams;
    FContactInfo LastContact;
    FVector ArcVelocity;        // Velocity of the Ball when it is being "animated" on an Arc.
    FTimerHandle ToNeutralTimer;
    bool bCaptureAllowed = true;

public:	
	// Sets default values for this actor's properties
	ASoccerBall();

    void DisablePhysics();
    void PreventCapture();
    bool Capture(AOutfieldCharacter* pCharacter);
    //float KickToward(FVector targetLocation, const FVector& targetVelocity, const bool bReleaseTeamPossession = false, float speedMultiplier = 1.f);

    float KickToTarget(const FKickParams& params);

    TObjectPtr<AOutfieldCharacter> GetPlayerInPossession() const { return PlayerInPossession; }
    FContactInfo& GetLastContactInfo() { return LastContact; }
    //float GetBallRadius() const { return SphereCollider->GetScaledSphereRadius(); }

protected:
	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode) override;
    virtual void OnMatchPhaseBegin(const EMatchPhase phase) override;
    void OnRequestMatchPlayState(const TObjectPtr<AProjectSoccerGameMode>& pGameMode, const EMatchPhase requestedState);

    void BeginNeutralTimer(const float time);
    void OnNeutralTimerComplete();
    void OnArcMovementFinished() const;
    void CancelNeutralTimer();

    bool HandleArcFinished() const;

    static float CalculateTimeToReachTarget(const FVector& targetLocation, const FVector& targetVelocity, float speed);
};