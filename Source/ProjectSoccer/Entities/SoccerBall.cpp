// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectSoccer/Entities/SoccerBall.h"

#include "Components/InterpToMovementComponent.h"
#include "Outfielder/OutfieldCharacter.h"
#include "Components/SphereComponent.h"
#include "Field/ArcMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectSoccer/GameMode/ProjectSoccerGameMode.h"
#include "Windows/AllowWindowsPlatformTypes.h"

DEFINE_LOG_CATEGORY(LogSoccerBall);

ASoccerBall::ASoccerBall()
{
    // Sphere Collider (Root)
    SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollider"));
    RootComponent = SphereCollider;

    // Mesh
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(SphereCollider);

    // Movement Component
    ArcMovementComponent = CreateDefaultSubobject<UArcMovementComponent>(TEXT("ArcMovementComponent"));
    ArcMovementComponent->SetUpdatedComponent(SphereCollider);
    ArcMovementComponent->OnArcMovementFinished().AddUObject(this, &ASoccerBall::OnArcMovementFinished);
    ArcMovementComponent->OnArcMovementFinishedOrInterrupted().BindUObject(this, &ASoccerBall::HandleArcFinished);
}

void ASoccerBall::BeginPlay()
{
	Super::BeginPlay();
    // Register for Phase Requests.

    AProjectSoccerGameMode::OnMatchInit().AddUObject(this, &ASoccerBall::OnMatchInit);
    AProjectSoccerGameMode::OnMatchPhaseBegin().AddUObject(this, &ASoccerBall::OnMatchPhaseBegin);
    AProjectSoccerGameMode::AddOnRequestMatchPhaseListener(this, &ASoccerBall::OnRequestMatchPlayState);
}

void ASoccerBall::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // UnRegister for GameMode Events.
    AProjectSoccerGameMode::OnMatchInit().RemoveAll(this);
    AProjectSoccerGameMode::OnMatchPhaseBegin().RemoveAll(this);
    AProjectSoccerGameMode::RemoveOnRequestMatchPhaseListener(this);
}

void ASoccerBall::OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode)
{
    GameMode = pGameMode;
}

void ASoccerBall::OnMatchPhaseBegin(const EMatchPhase phase)
{
    switch (phase)
    {
    	case EMatchPhase::Playing:
		{
            bCaptureAllowed = true;
            DisablePhysics();
			SphereCollider->SetSimulatePhysics(true);
            break;
        }

    	default: break;
    }
}

void ASoccerBall::OnRequestMatchPlayState(const TObjectPtr<AProjectSoccerGameMode>& pGameMode, const EMatchPhase requestedState)
{
    // If we are requesting to play, then stop the ball and reset its position.
    if (requestedState == EMatchPhase::Kickoff)
    {
        CancelNeutralTimer();
        if (PlayerInPossession)
        {
            PlayerInPossession->RemoveBallPossession();
            PlayerInPossession = nullptr;
        }

        DisablePhysics();
        SetActorLocation(FVector(0.f, 0.f, 50.f), false, nullptr, ETeleportType::TeleportPhysics);
    }

    else if (requestedState == EMatchPhase::PostMatch)
    {
        CancelNeutralTimer();
    }

    GameMode->NotifyReadyForNextPlayState();
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Disables the Physics of the Ball, and stops any movement.
//----------------------------------------------------------------------------------------------------
void ASoccerBall::DisablePhysics()
{
    UE_LOG(LogSoccerBall, Log, TEXT("Disabling Ball Physics and canceling Arc."));
    ArcMovementComponent->CancelArc();
    SphereCollider->SetSimulatePhysics(false);
}

void ASoccerBall::PreventCapture()
{
    bCaptureAllowed = false;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Capturing the Ball will update the Possession State of the Game.
///		@param pCharacter : Outfielder that is capturing the ball.
///		@returns : False if Capture is not allowed or the Character is already in possession or null.
//----------------------------------------------------------------------------------------------------
bool ASoccerBall::Capture(AOutfieldCharacter* pCharacter)
{
    if (!bCaptureAllowed || pCharacter == nullptr || pCharacter == PlayerInPossession)
        return false;

    CancelNeutralTimer();
    //DisablePhysics();

    if (PlayerInPossession)
    {
	    PlayerInPossession->RemoveBallPossession();
    }

    PlayerInPossession = pCharacter;

    LastContact.Outfielder = pCharacter;
    LastContact.Location = GetActorLocation();
    //LastContact.Type = FContactInfo::EType::Dribble;

    // Notify the GameMode that the ball possession has changed.
    GameMode->BallPossessionChanged(PlayerInPossession);
    
    return true;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      TODO: Should this check to see if we have a valid player in possession?
//		
///		@brief : Execute a Kick of the Ball, sending it on an Arc to a Target.
///		@param params : Parameters to describe the Kick, it's target and movement characteristics.
///		@returns : An approximate time that it will take for the Ball to reach the Target.
//----------------------------------------------------------------------------------------------------
float ASoccerBall::KickToTarget(const FKickParams& params)
{
    // Constant that adds on a little bit of time to the neutral timer after kicking the ball.
    static constexpr float kAdditionalNeutralTimerTime = 0.025f;

    CancelNeutralTimer();

    // Set the ball location's Z to match the target, so that we only pass in 2D.
    const FVector ballLocation = GetActorLocation();
    //params.TargetLocation.Z = ballLocation.Z;

    // Record the last Contact Info.
    LastContact.Location = ballLocation;
    LastContact.Outfielder = PlayerInPossession;
    LastContact.Type = FContactInfo::EType::Kick;

    PlayerInPossession = nullptr;

	FMoveAlongArcParams arcParams;
    arcParams.StartLocation = ballLocation;
    arcParams.TargetLocation = params.TargetLocation;
    arcParams.InitialVelocity = params.TargetVelocity;
    arcParams.MaxHeightOffset = params.MaxArcHeightOffset;
    arcParams.VelocityCurve = params.VelocityCurve;
    arcParams.Speed = params.Speed;

    float timeToHitTarget;
    ArcMovementComponent->BeginMoveAlongArc(arcParams, timeToHitTarget);

    // If shooting, complete the possession timer immediately.
    if (params.Type == FKickParams::EType::Shot)
        OnNeutralTimerComplete();
    // Begin a timer to the timeToHit, so that we can track if that hit fails, we need to notify that it is now a neutral space.
    else
		BeginNeutralTimer(timeToHitTarget + kAdditionalNeutralTimerTime);

    bCaptureAllowed = true;
    return timeToHitTarget;
}

// TODO: Get Rid
//float ASoccerBall::KickToward(FVector targetLocation, const FVector& targetVelocity, const bool bReleaseTeamPossession, float speedMultiplier)
//{
//    // Constant that adds on a little bit of time to the neutral timer after kicking the ball.
//    static constexpr float kAdditionalNeutralTimerTime = 0.025f;
//
//    CancelNeutralTimer();
//    //DisablePhysics();
//
//    // Set the ball location's Z to match the target, so that we only pass in 2D.
//    FVector ballLocation = GetActorLocation();
//    targetLocation.Z = ballLocation.Z;
//    //SetActorLocation(ballLocation, false, nullptr, ETeleportType::ResetPhysics);
//
//    // Record the last Contact Info.
//    LastContact.Location = ballLocation;
//    LastContact.Outfielder = PlayerInPossession;
//    LastContact.Type = FContactInfo::EType::Shot;
//
//	PlayerInPossession = nullptr;
//    const float speed = BaseSpeed * speedMultiplier;
//    float timeToHitTarget = CalculateTimeToReachTarget(targetLocation, targetVelocity, speed);
//    const FVector adjustedTargetLocation = targetLocation + targetVelocity * timeToHitTarget;
//
//    FVector direction = adjustedTargetLocation - ballLocation;
//    direction.Normalize();
//
//    FMoveAlongArcParams arcParams;
//    arcParams.StartLocation = ballLocation;
//    arcParams.TargetLocation = targetLocation;
//    arcParams.TargetVelocity = targetVelocity;
//    arcParams.MaxHeightOffset = 1.f; // Tune this for passing...
//    //arcParams.DeltaHeight = -0.005f;  // Barely any gravity, trying to skate the ball along the ground?
//    //arcParams.EndLocation = adjustedTargetLocation;
//    //arcParams.Duration = timeToHitTarget;
//    //arcParams.UpAngleRad = 0.f;
//    //arcParams.RightAngleRad = 0.f;
//    arcParams.Speed = speed;
//
//    ArcMovementComponent->BeginMoveAlongArc(arcParams, timeToHitTarget);
//
//    // If releasing team possession, complete the possession timer immediately.
//    if (bReleaseTeamPossession)
//        OnNeutralTimerComplete();
//    // Begin a timer to the timeToHit, so that we can track if that hit fails, we need to notify that it is now a neutral space.
//    else
//		BeginNeutralTimer(timeToHitTarget + kAdditionalNeutralTimerTime);
//
//    bCaptureAllowed = true;
//
//    return timeToHitTarget;
//}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Begin a timer that will notify the GameMode that the Ball is not in possession of either
///             Team when completed.
///		@param time : The amount of time to wait before the timer completes, in seconds.
//----------------------------------------------------------------------------------------------------
void ASoccerBall::BeginNeutralTimer(const float time)
{
	GetWorldTimerManager().SetTimer(ToNeutralTimer, this, &ASoccerBall::OnNeutralTimerComplete, 0.1f, false, time);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : When the neutral timer completes, we will notify the GameMode that neither Team
///             is in possession of the ball.
//----------------------------------------------------------------------------------------------------
void ASoccerBall::OnNeutralTimerComplete()
{
	CancelNeutralTimer();
    GameMode->BallPossessionChanged(nullptr);
}

void ASoccerBall::OnArcMovementFinished() const
{
    // If we aren't in possession, then we can turn physics back on.
    if (!PlayerInPossession)
    {
        UE_LOG(LogSoccerBall, Log, TEXT("Arc Finished, no player, turning Physics on."));
        //ArcMovementComponent->SetUpdatedComponent(nullptr);
		SphereCollider->SetSimulatePhysics(true);
    }
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Cancel the Neutral Timer, which will prevent the Possession State to become Neutral.
//----------------------------------------------------------------------------------------------------
void ASoccerBall::CancelNeutralTimer()
{
	if (ToNeutralTimer.IsValid())
	{
		GetWorldTimerManager().ClearTimer(ToNeutralTimer);
	}
}

bool ASoccerBall::HandleArcFinished() const
{
    return PlayerInPossession == nullptr;
}

float ASoccerBall::CalculateTimeToReachTarget(const FVector& targetLocation, const FVector& targetVelocity, float speed)
{
    // https://playtechs.blogspot.com/2007/04/aiming-at-moving-target.html
	const float a = speed * speed - (FVector::DotProduct(targetVelocity, targetVelocity));
    const float b = FVector::DotProduct(targetLocation, targetVelocity);
    const float c = FVector::DotProduct(targetLocation, targetLocation);

    const float discriminant = b * b + a * c;

    float time = 0.f;
    if (discriminant >= 0)
    {
	    time = (b + FMath::Sqrt(discriminant)) / a;

        // If the discriminant is less than zero, then Ball cannot catch up to the target.
        time = FMath::Max(time, 0.f);
    }

    return time;
}