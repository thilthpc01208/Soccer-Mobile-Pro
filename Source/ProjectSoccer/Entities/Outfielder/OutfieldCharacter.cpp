// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/Entities/Outfielder/OutfieldCharacter.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectSoccer/Entities/Field/GoalTrigger.h"
#include "ProjectSoccer/Entities/GameCamera.h"
#include "ProjectSoccer/Entities/SoccerBall.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"
#include "ProjectSoccer/GameMode/ProjectSoccerGameMode.h"
#include "ProjectSoccer/Tools/DebugViews/DebugViewManager.h"
#include "ProjectSoccer/UI/Outfielder/BillboardWidgetComponent.h"
#include "ProjectSoccer/UI/Outfielder/OutfielderControlIcon.h"
#include "ProjectSoccer/UI/Outfielder/OutfielderDebugStatsWidget.h"
#include "ProjectSoccer/UI/Outfielder/OutfielderInputWidget.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_DEBUG_VIEW_TARGET_TAG(DV_StatsWidget, "Outfielder.Stats");
DEFINE_LOG_CATEGORY(LogOutfieldCharacter)

namespace
{
const FName PlayerIconComponentName(TEXT("PlayerIcon Widget"));
const FName InputWidgetComponentName(TEXT("Input Widget"));
const FName DebugStatsWidgetComponentName(TEXT("Debug Stats Widget"));

const TCHAR* PlayerIconWidgetAssetPath = TEXT("/Game/ProjectSoccer/UI/Outfielder/WB_OutfielderControlIcon");
const TCHAR* InputWidgetAssetPath = TEXT("/Game/ProjectSoccer/UI/Outfielder/WB_OutfielderInput");
const TCHAR* StatsWidgetAssetPath = TEXT("/Game/ProjectSoccer/UI/Outfielder/WB_OutfielderStats");

const TCHAR* PlayerIconWidgetClassPath = TEXT("/Game/ProjectSoccer/UI/Outfielder/WB_OutfielderControlIcon.WB_OutfielderControlIcon_C");
const TCHAR* InputWidgetClassPath = TEXT("/Game/ProjectSoccer/UI/Outfielder/WB_OutfielderInput.WB_OutfielderInput_C");
const TCHAR* StatsWidgetClassPath = TEXT("/Game/ProjectSoccer/UI/Outfielder/WB_OutfielderStats.WB_OutfielderStats_C");

template <typename TComponent>
TComponent* FindOwnedComponentByName(const AActor* Owner, const FName ComponentName)
{
    TArray<TComponent*> Components;
    Owner->GetComponents<TComponent>(Components);

    for (TComponent* Component : Components)
    {
        if (IsValid(Component) && Component->GetFName() == ComponentName)
        {
            return Component;
        }
    }

    return nullptr;
}

template <typename TComponent>
TComponent* EnsureOwnedComponent(AActor* Owner, TObjectPtr<TComponent>& Component, const FName ComponentName, USceneComponent* AttachParent)
{
    if (!IsValid(Component))
    {
        Component = FindOwnedComponentByName<TComponent>(Owner, ComponentName);
    }

    if (!IsValid(Component))
    {
        Component = NewObject<TComponent>(Owner, ComponentName);
        Component->SetupAttachment(AttachParent);
        Component->RegisterComponent();
    }

    return Component;
}

TSubclassOf<UUserWidget> ResolveWidgetClass(const TCHAR* ClassPath)
{
    return StaticLoadClass(UUserWidget::StaticClass(), nullptr, ClassPath);
}

void EnsureWidgetComponentDefaults(UWidgetComponent* WidgetComponent, const TSubclassOf<UUserWidget>& WidgetClass)
{
    if (!IsValid(WidgetComponent))
    {
        return;
    }

    if (WidgetClass && !WidgetComponent->GetWidgetClass())
    {
        WidgetComponent->SetWidgetClass(WidgetClass);
    }

    if (!WidgetComponent->GetWidget() && WidgetComponent->GetWidgetClass())
    {
        WidgetComponent->InitWidget();
    }
}
}

// Sets default values
AOutfieldCharacter::AOutfieldCharacter()
{
    // Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    BallCaptureCapsule = CreateDefaultSubobject<UCapsuleComponent>("BallCaptureCapsule");
    BallCaptureCapsule->SetupAttachment(GetCapsuleComponent());
    BallCaptureCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
    BallCaptureCapsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECR_Overlap);
    // TODO: This might have different behavior that the Capture Sphere, because I want there to be a
    // threshold of Speed on the Capsule collision so that when the Ball is shot, it bounces off the Player
    // body instead of being captured here.
    BallCaptureCapsule->OnComponentBeginOverlap.AddDynamic(this, &AOutfieldCharacter::OnBeginOverlap);

    // Set up a target for our when we have possession of the ball.
    BallPossessionTarget = CreateDefaultSubobject<USceneComponent>("BallPossessionTarget");
    BallPossessionTarget->SetupAttachment(GetCapsuleComponent());

    // Set up another sphere for capturing the ball when it is near the BallPossessionTarget.
    BallCaptureSphere = CreateDefaultSubobject<USphereComponent>("BallCaptureSphere");
    BallCaptureSphere->SetupAttachment(GetCapsuleComponent());
    BallCaptureSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    BallCaptureSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECR_Overlap);
    BallCaptureSphere->OnComponentBeginOverlap.AddDynamic(this, &AOutfieldCharacter::OnBeginOverlap);

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

    // Configure character movement
    auto* pCharacterMovement = GetCharacterMovement();
	pCharacterMovement->bOrientRotationToMovement = true; // Rotate character to moving direction
	pCharacterMovement->RotationRate = FRotator(0.f, 640.f, 0.f);
	pCharacterMovement->bConstrainToPlane = true;
	pCharacterMovement->bSnapToPlaneAtStart = true;
    pCharacterMovement->bOrientRotationToMovement = true;

    // Player Icon
    PlayerIcon = CreateDefaultSubobject<UBillboardWidgetComponent>(PlayerIconComponentName);
    PlayerIcon->SetupAttachment(GetCapsuleComponent());
    static ConstructorHelpers::FClassFinder<UUserWidget> PlayerIconWidgetClass(PlayerIconWidgetAssetPath);
    if (PlayerIconWidgetClass.Succeeded())
    {
        PlayerIcon->SetWidgetClass(PlayerIconWidgetClass.Class);
    }

    // Input Widget
    InputWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(InputWidgetComponentName);
    InputWidgetComponent->SetupAttachment(GetCapsuleComponent());
    InputWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
    static ConstructorHelpers::FClassFinder<UUserWidget> InputWidgetClass(InputWidgetAssetPath);
    if (InputWidgetClass.Succeeded())
    {
        InputWidgetComponent->SetWidgetClass(InputWidgetClass.Class);
    }

    //-----------------------------------
    // TODO: Debug Only:
    DebugStatsWidgetComponent = CreateDefaultSubobject<UBillboardWidgetComponent>(DebugStatsWidgetComponentName);
    DebugStatsWidgetComponent->SetupAttachment(GetCapsuleComponent());
    DebugStatsWidgetComponent->SetVisibility(false);
    static ConstructorHelpers::FClassFinder<UUserWidget> StatsWidgetClass(StatsWidgetAssetPath);
    if (StatsWidgetClass.Succeeded())
    {
        DebugStatsWidgetComponent->SetWidgetClass(StatsWidgetClass.Class);
    }
    //-----------------------------------

    OnPlayerGainedPossessionEvent.AddUObject(this, &AOutfieldCharacter::OnGainedPossession);

    REGISTER_DEBUG_VIEW_TARGET_FUNC(DV_StatsWidget, this, &AOutfieldCharacter::ShowStats);
}

void AOutfieldCharacter::BeginPlay()
{
    Super::BeginPlay();

    EnsureOwnedComponent(this, PlayerIcon, PlayerIconComponentName, GetCapsuleComponent());
    EnsureOwnedComponent(this, InputWidgetComponent, InputWidgetComponentName, GetCapsuleComponent());
    EnsureOwnedComponent(this, DebugStatsWidgetComponent, DebugStatsWidgetComponentName, GetCapsuleComponent());

    if (InputWidgetComponent)
    {
        InputWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
    }

    static const TSubclassOf<UUserWidget> RuntimePlayerIconWidgetClass = ResolveWidgetClass(PlayerIconWidgetClassPath);
    static const TSubclassOf<UUserWidget> RuntimeInputWidgetClass = ResolveWidgetClass(InputWidgetClassPath);
    static const TSubclassOf<UUserWidget> RuntimeStatsWidgetClass = ResolveWidgetClass(StatsWidgetClassPath);

    EnsureWidgetComponentDefaults(PlayerIcon, RuntimePlayerIconWidgetClass);
    EnsureWidgetComponentDefaults(InputWidgetComponent, RuntimeInputWidgetClass);
    EnsureWidgetComponentDefaults(DebugStatsWidgetComponent, RuntimeStatsWidgetClass);

    // Apply the default Movement Config to the Outfielder.
    ApplyMovementConfig(BaseConfig);

    // Cache the starting position of the BallPossessionTarget, so that we can reset it when we have possession of the Ball.
    DefaultLocalPossessionTargetLocation = BallPossessionTarget != nullptr
        ? BallPossessionTarget->GetRelativeLocation()
        : FVector::ZeroVector;

    if (IsValid(InputWidgetComponent))
    {
        InputWidget = Cast<UOutfielderInputWidget>(InputWidgetComponent->GetUserWidgetObject());
        InitialInputWidgetRotation = InputWidgetComponent->GetComponentRotation();
    }

    if (!InputWidget)
    {
        UE_LOG(LogOutfieldCharacter, Warning, TEXT("%s is missing a valid InputWidget on '%s'."), *GetName(), *InputWidgetComponentName.ToString());
    }

    //------------------------------
	// TODO: Debug Only
    if (IsValid(DebugStatsWidgetComponent))
    {
        DebugStatsWidgetComponent->SetVisibility(false);
        StatsWidget = Cast<UOutfielderDebugStatsWidget>(DebugStatsWidgetComponent->GetUserWidgetObject());
    }

    if (!StatsWidget)
    {
        UE_LOG(LogOutfieldCharacter, Warning, TEXT("%s is missing a valid DebugStats widget on '%s'."), *GetName(), *DebugStatsWidgetComponentName.ToString());
    }
    //------------------------------

    // Register for GameMode Events.
    AProjectSoccerGameMode::OnMatchInit().AddUObject(this, &AOutfieldCharacter::OnMatchInit);
    AProjectSoccerGameMode::OnMatchPhaseBegin().AddUObject(this, &AOutfieldCharacter::OnMatchPhaseBegin);
}

void AOutfieldCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // UnRegister for GameMode Events.
    AProjectSoccerGameMode::OnMatchInit().RemoveAll(this);
    AProjectSoccerGameMode::OnMatchPhaseBegin().RemoveAll(this);
}

void AOutfieldCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (bMovementVectorLocked)
	{
		GetCharacterMovement()->Velocity = LockedVelocity;
	}

    // Update the Charge Curve
    if (bIsCharging)
    {
        check(ChargeCurve);

        // Increase the time, but max out at 1.f
        CurrentChargeTime += DeltaSeconds;
        if (CurrentChargeTime > ChargeMaxTime)
            CurrentChargeTime = ChargeMaxTime;

        CurrentCharge = ChargeCurve->GetFloatValue(CurrentChargeTime / ChargeMaxTime);
        if (InputWidget)
        {
            InputWidget->SetCharge(CurrentCharge);
        }
    }

    CalculatePressure();
    CalculateThreat();

    //------------------------------
	// TODO: Debug Only  
    UpdateStatsWidget();
    //------------------------------
}

void AOutfieldCharacter::OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode)
{
    GameCamera = pGameMode->GetGameCamera();

    // Set the Game Camera for each Billboard Widget:
    if (PlayerIcon)
    {
        PlayerIcon->SetGameCamera(GameCamera);
    }

    if (DebugStatsWidgetComponent)
    {
        DebugStatsWidgetComponent->SetGameCamera(GameCamera);
    }
}

void AOutfieldCharacter::OnMatchPhaseBegin(const EMatchPhase phase)
{
    switch (phase)
    {
    	case EMatchPhase::Kickoff:
    	{
            OnReset();
    		break;
    	}

    	default: break;
    }
}

void AOutfieldCharacter::SetOutfielderInfo(const FOutfielderCreateInfo& info)
{
    TeamManager = info.TeamManager;
    OutfielderId = info.Id;
    bOnPlayerTeam = info.bPlayerTeam;
	GetMesh()->SetMaterialByName(TeamMaterialSlotName, info.TeamMaterial);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Set the Icon info for the Outfielder Controlled by a Player.
///		@param index : Index of the PlayerController.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::SetControlledByPlayer(const int index) const
{
    if (!PlayerIcon)
    {
        UE_LOG(LogOutfieldCharacter, Warning, TEXT("%s cannot show a player icon because PlayerIcon is missing."), *GetName());
        return;
    }

    if (UOutfielderControlIcon* pIcon = Cast<UOutfielderControlIcon>(PlayerIcon->GetWidget()))
    {
        pIcon->SetPlayerIcon(index);
        return;
    }

    UE_LOG(LogOutfieldCharacter, Warning, TEXT("%s is missing WB_OutfielderControlIcon on PlayerIcon."), *GetName());
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Set the Icon info for the Outfielder Controlled by a CPU.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::SetControlledIconCPU() const
{
    if (!PlayerIcon)
    {
        UE_LOG(LogOutfieldCharacter, Warning, TEXT("%s cannot show a CPU icon because PlayerIcon is missing."), *GetName());
        return;
    }

    if (UOutfielderControlIcon* pIcon = Cast<UOutfielderControlIcon>(PlayerIcon->GetWidget()))
    {
        pIcon->SetCPUIcon();
        return;
    }

    UE_LOG(LogOutfieldCharacter, Warning, TEXT("%s is missing WB_OutfielderControlIcon on PlayerIcon."), *GetName());
}

void AOutfieldCharacter::Move(const FVector2D movementVector)
{
    if (bCurrentActionStopsMovement)
        return;

    // If our movement vector is not locked.
    if (!bMovementVectorLocked)
		MovementVector = { movementVector.Y, movementVector.X, 0};

	AddMovementInput(MovementVector, 1.0, false);

    // If we have possession of the Ball, update the Input Widget's Direction.
    if (HasPossessionOfBall())
    {
        if (InputWidget)
        {
            InputWidget->SetDirection(MovementVector);
        }

        // Rotate to match the 
        FRotator rotation = InitialInputWidgetRotation;
        // What is the Yaw to add?
        const float yaw = FMath::RadiansToDegrees(FMath::Atan2(MovementVector.Y, MovementVector.X));
        rotation.Add(0.f, yaw, 0.f);
        if (InputWidgetComponent)
        {
            InputWidgetComponent->SetWorldRotation(rotation);
        }
    }
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      TODO: When/if you have the "Locking on" mechanic, that should be a second parameter.
//		
///		@brief : Perform a pass in the direction of the input movement vector.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::Pass()
{
    // If we are not moving in any direction, then pass to the closest teammate.
    if (GetMovementComponent()->Velocity.IsNearlyZero())
    {
	    auto pClosestTeammate = TeamManager->GetClosestTeammate(OutfielderId);
        PassToTeammate(pClosestTeammate);
        return;
    }

    const auto& fieldState = TeamManager->GetFieldState();

	// [TODO]: Expose as a boolean in the editor, but only use this within an editor context.
	// It shouldn't ship.
#define FORCE_PASS_TO_CLOSEST_TEAMMATE 1
#if FORCE_PASS_TO_CLOSEST_TEAMMATE
	const auto id = fieldState.GetClosestTeammate(OutfielderId);
	if (!id.IsValid())
		return;
	
	// Pass to the Closest Teammate;
	TObjectPtr<AOutfieldCharacter> pClosestTeammate = TeamManager->GetOutfielder(id);
	PassToTeammate(pClosestTeammate);

#else
    const float threshold = 1.f - (PassAimAngleThreshold / 90.f);
    // TODO: Should I also consider a distance based on the current charge?
    // I can calculate the projected distance based on the current charge, and then make a Radius around that point based on the
    // angle threshold. Then the closest teammate within that radius will be the one we pass to.
    const auto id = fieldState.GetClosestTeammatePassInDirection(OutfielderId, MovementVector.GetSafeNormal(), threshold);

	// If there is a valid teammate within the threshold, pass to the teammate.
	if (id.IsValid())
	{
		// Pass to the Closest Teammate;
		TObjectPtr<AOutfieldCharacter> pClosestTeammate = TeamManager->GetOutfielder(id);
		PassToTeammate(pClosestTeammate);
	}

	// Otherwise, Pass in the direction:
	else
	{
		//PassInCurrentDirection();
	}
#endif
	// [TODO]: Right now, I am forcing Pass to Closest.
}

//----------------------------------------------------------------------------------------------------
///		@brief : Pass to a Teammate.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::PassToTeammate(TObjectPtr<AOutfieldCharacter> pTeammate)
{
	if (HasPossessionOfBall() && pTeammate)
	{
        Ball->PreventCapture();

        FKickParams kickParams;
        kickParams.Type = FKickParams::EType::Pass;

		// Force passing directly to the teammate, to debug.
		// [TODO]: Expose as a boolean in the editor, but only use this within an editor context.
		// It shouldn't ship.
#define FORCE_PASS_DIRECTLY_TO_TEAMMATE 1
#if FORCE_PASS_DIRECTLY_TO_TEAMMATE
		kickParams.TargetLocation = pTeammate->GetActorLocation();
		kickParams.TargetLocation.Z = pTeammate->GetPassTargetLocation().Z;
#else
        kickParams.TargetLocation = pTeammate->GetPassTargetLocation();
#endif
		
        kickParams.TargetVelocity = pTeammate->GetCharacterMovement()->GetLastUpdateVelocity();
        kickParams.MaxArcHeightOffset = PassArcHeight;
        kickParams.VelocityCurve = PassVelocityCurve;
        kickParams.Speed = FMath::Lerp(PassMinSpeed, PassMaxSpeed, CurrentCharge);

		// TODO: Implement Arc Curve based on difference in direction of the input value and the outfielder position.
        kickParams.ArcCurve = 0.f;

        //const FVector teammateLocation = pTeammate->GetActorLocation();
        //const FVector teammateVelocity = pTeammate->GetCharacterMovement()->GetLastUpdateVelocity();;

        // Hack: Immediately stop and face the player being passed to.
		FVector toTeammate = kickParams.TargetLocation - GetActorLocation();
		toTeammate.Normalize();
        GetCharacterMovement()->StopMovementImmediately();
        SetActorRotation(toTeammate.Rotation(), ETeleportType::ResetPhysics);

        //const float timeToReachTarget = Ball->KickToward(teammateLocation, teammateVelocity);
        const float timeToReachTarget = Ball->KickToTarget(kickParams);

        // Lock the teammate's movement vector, so that the pass will be received.
        pTeammate->LockMovementForDuration(timeToReachTarget);

		RemoveBallPossession();
	}

	EndCharge();
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Pass the ball in the current Forward direction. The distance is based on the
///             CurrentCharge of the Pass.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::PassInCurrentDirection()
{
    if (HasPossessionOfBall())
    {
        Ball->PreventCapture();

        FKickParams kickParams;
        kickParams.Type = FKickParams::EType::Pass;

        const FVector targetLocation = GetActorLocation() + GetActorForwardVector() * (FMath::Lerp(ManualPassDistOnMinCharge, ManualPassDistOnMaxCharge, CurrentCharge));
        kickParams.TargetLocation = targetLocation;
        kickParams.TargetVelocity = FVector::Zero();
        kickParams.MaxArcHeightOffset = PassArcHeight;
        kickParams.VelocityCurve = PassVelocityCurve;
        kickParams.Speed = FMath::Lerp(PassMinSpeed, PassMaxSpeed, CurrentCharge);

        // TODO: 
        kickParams.ArcCurve = 0.f;

        Ball->KickToTarget(kickParams);
        RemoveBallPossession();
    }

    EndCharge();
}


//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Perform a Shot. If the Outfielder is not in Possession of the Ball, then this will
///              do nothing. If the Outfielder is not in the Opponent's Half, then this will do nothing.
///		@param movementVector : 
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::Shoot(const FVector2D movementVector)
{
    // TODO: This shouldn't occur, change to an assert once you have the Tackle behavior in.
	if (!HasPossessionOfBall())
	{
        return;
	}

    // Shoot if in possession
    // If not in the Team Half, then clear the ball?
    if (TeamManager->OutfielderInTeamHalf(this))
    {
		// TODO: Clear the ball
        EndCharge();
    	return;
    }

    Ball->PreventCapture();
    const FVector current = GetActorLocation();
    const FVector movementVec3 = { movementVector.Y, movementVector.X, 0 };

    FKickParams kickParams;
    kickParams.Type = FKickParams::EType::Shot;
    kickParams.TargetLocation = GetShotLocation(movementVec3, CurrentCharge);
    kickParams.TargetVelocity = FVector::Zero();
    kickParams.MaxArcHeightOffset = ShotArcHeight;

	// TODO: Multiplier...
    kickParams.Speed = FMath::Lerp(ShotMinSpeed, ShotMaxSpeed, CurrentCharge);

	// TODO: Implement Arc Curve based on difference in direction of the input value and the outfielder position.
    kickParams.ArcCurve = 0.f;
    bCurrentActionStopsMovement = true;

    // Hack: Immediately Face the Shot Direction.
    GetCharacterMovement()->StopMovementImmediately();

    // Clamp the face direction's Z value to the current Z value.
    FVector facePosition = kickParams.TargetLocation;
    facePosition.Z = GetActorLocation().Z;

    FVector toGoal = facePosition - current;
    toGoal.Normalize();
    SetActorRotation(toGoal.Rotation(), ETeleportType::ResetPhysics);

    // Kick the Ball
    Ball->KickToTarget(kickParams);
	RemoveBallPossession();
    EndCharge();
    bCurrentActionStopsMovement = false;
}

void AOutfieldCharacter::Tackle(const FVector2D& movementVector)
{
	// TODO: 
}

//----------------------------------------------------------------------------------------------------
///		@brief : Begin Charging a Shot.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::BeginChargeShot()
{
	BeginCharge();

	// Stop the Outfielder movement.
	bCurrentActionStopsMovement = true;

    // TODO: Have a delegate for the Shot function, so that when the charge is at full,
    // we automatically shoot.
}

//----------------------------------------------------------------------------------------------------
///		@brief : Begin charging a pass.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::BeginChargePass()
{
	BeginCharge();

    // TODO: Have a delegate for the Pass function, so that when the charge is at full,
    // we automatically pass.
}

void AOutfieldCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    //GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, TEXT("Overlapped!"));
    auto* pBallActor = Cast<ASoccerBall>(OtherActor);
    if (bCanCaptureBall && pBallActor != nullptr && !PossessionLockedTimerHandle.IsValid())
    {
        // Attempt to capture the ball.
        if (!pBallActor->Capture(this))
            return;

        Ball = pBallActor;
        Ball->DisablePhysics();

        // Reset the Ball Possession Target's location.
        BallPossessionTarget->SetRelativeLocation(DefaultLocalPossessionTargetLocation);

		const FAttachmentTransformRules rules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, true);
		OtherActor->AttachToComponent(BallPossessionTarget, rules);
        bCanCaptureBall = false;

        UE_LOG(LogOutfieldCharacter, Log, TEXT("Captured ball, attaching!"));

        // Show the Input Widget:
        if (InputWidget)
        {
            InputWidget->ShowDirection(MovementVector);
        }

        // Broadcast the event:
	    OnPlayerGainedPossessionEvent.Broadcast(this);
    }
}

void AOutfieldCharacter::RemoveBallPossession()
{
    if (Ball != nullptr)
    {
	    const FDetachmentTransformRules rules(EDetachmentRule::KeepWorld, true);
		Ball->DetachFromActor(rules);
		Ball = nullptr;

        UE_LOG(LogOutfieldCharacter, Log, TEXT("Removing Ball possession, detaching!"));
		StartPossessionCooldown();

        if (InputWidget)
        {
            InputWidget->HideCompletely();
        }
    	bCurrentActionStopsMovement = false;
    }
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Begin a timer to prevent this Outfielder from capturing the ball. This is used to
///             prevent the Outfielder from immediately capturing the ball after a pass or shot.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::StartPossessionCooldown()
{
	GetWorldTimerManager().SetTimer(PossessionLockedTimerHandle, this, &AOutfieldCharacter::FinishPossessionLockTimer, 0.1f, false, PossessionCountdownTime);
}

void AOutfieldCharacter::FinishPossessionLockTimer()
{
    //GEngine->AddOnScreenDebugMessage(2, 3.f, FColor::Green, TEXT("Finished Timer."));
    bCanCaptureBall = true;

    if (PossessionLockedTimerHandle.IsValid())
		GetWorldTimerManager().ClearTimer(PossessionLockedTimerHandle);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Lock the current movement vector of the Outfielder. Locking the movement vector
///           will prevent the Outfielder from changing their current movement
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::LockMovement()
{
	if (MovementLockTimerHandle.IsValid())
		GetWorldTimerManager().ClearTimer(MovementLockTimerHandle);

    bMovementVectorLocked = true;
    LockedVelocity = GetCharacterMovement()->GetLastUpdateVelocity();
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Release the Lock on the movement vector of the Outfielder.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::ReleaseMovement()
{
	FinishMovementLockTimer();
}

void AOutfieldCharacter::LockMovementForDuration(const float duration)
{
    LockedVelocity = GetCharacterMovement()->GetLastUpdateVelocity();
	bMovementVectorLocked = true;
    GetWorldTimerManager().SetTimer(MovementLockTimerHandle, this, &AOutfieldCharacter::FinishMovementLockTimer, 0.1f, false, duration);
}

void AOutfieldCharacter::FinishMovementLockTimer()
{
	bMovementVectorLocked = false;

    if (MovementLockTimerHandle.IsValid())
		GetWorldTimerManager().ClearTimer(MovementLockTimerHandle);
}

void AOutfieldCharacter::OnReset()
{
    bMovementVectorLocked = false;
    bCanCaptureBall = true;
    RemoveBallPossession();
    EndCharge();
}

void AOutfieldCharacter::OnGainedPossession(AOutfieldCharacter* pOutfielder)
{
    FinishMovementLockTimer();
}

FVector AOutfieldCharacter::GetPassTargetLocation() const
{
    const float velocitySquared = GetMovementComponent()->Velocity.SquaredLength();
    const float ratio = velocitySquared / FMath::Square(GetCharacterMovement()->MaxWalkSpeed);
    const float passDistance = ratio * MaxPassLeadDistance;
    const FVector footLocation = GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    return footLocation + GetActorForwardVector() * passDistance;

    //return BallPossessionTarget->GetComponentLocation();
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Calculate a shot location based on the shot direction and power. If the shot is aimed
///             past the ShotAimAngleThreshold, then the shot will be aimed past the goal in the direction
///             of the input shot direction.
///		@param shotDirection : Input direction of the shot.
///		@param shotCharge : Current charge of the Shot - Value from [0, 1].
//----------------------------------------------------------------------------------------------------
FVector AOutfieldCharacter::GetShotLocation(const FVector& shotDirection, const float shotCharge) const
{
    const FVector outfielderLocation = GetActorLocation();
    AGoalTrigger* pGoal = TeamManager->GetOpponentGoal();
    FVector goalLocation = pGoal->GetActorLocation();

	// Match the Z value so that we are dealing with the XY plane.
    goalLocation.Z = outfielderLocation.Z;

    // Get the direction to the Goal - this is to the bottom center of the Goal.
    const FVector toGoal = goalLocation - outfielderLocation;
    const FVector toCenterDirection = toGoal.GetSafeNormal();

    // Height is from 0 to the Goal Height, based on the shot charge.
    const float height = pGoal->GetGoalHeight() * shotCharge;

    // Get the angle between the shot direction and the direction to the goal.
    float angleDeg = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(shotDirection, toCenterDirection)));

#define ALLOW_SHOT_ERROR 0
    // If the input angle is past the threshold, then return a position pass the goal,
    // in the direction of the input.
    if (angleDeg > ShotAimAngleThreshold)
    {
#if ALLOW_SHOT_ERROR
        FVector result = outfielderLocation + shotDirection * (toGoal.Length() * 2.f);
        result.Z = height;
        return result;
#else
		angleDeg = ShotAimAngleThreshold;
#endif
    }

    // Calculate a vector between the left and right posts.
    const FVector leftPost = pGoal->GetLeftPost();
    const FVector toRightPost = pGoal->GetVectorFromLeftPostToRight();
    const FVector toRightDirection = toRightPost.GetSafeNormal();

    // Determine if the shot is to the left or right of the goal.
    float dot = FVector::DotProduct(shotDirection, toRightDirection);
    if (dot < 0)
        angleDeg = -angleDeg;

    // Value from [0, 1] with 0 representing the left post, and 1 representing the right post.
    const float normalizedToLeftPost = ((angleDeg / ShotAimAngleThreshold) + 1) * 0.5f;
    const FVector targetGoalLocation = leftPost + toRightPost * normalizedToLeftPost;

    // Calculate a Vector that passes through the target goal location, but is twice the distance from the outfielder.
    const FVector toTargetLocation = (targetGoalLocation - outfielderLocation);
    FVector result = outfielderLocation + (toTargetLocation * 2.f);
    result.Z = height;

    return result;
}

void AOutfieldCharacter::BeginCharge()
{
    bIsCharging = true;
    CurrentCharge = 0.f;
    CurrentChargeTime = 0.f;
	bCurrentActionStopsMovement = true;
    if (InputWidget)
    {
        InputWidget->ShowCharge(CurrentCharge);
    }
}

//----------------------------------------------------------------------------------------------------
///		@brief : Reset the CurrentCharge and the CurrentChargeTime, and set the IsCharging flag to false.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::EndCharge()
{
    bIsCharging = false;
    CurrentCharge = 0.f;
    CurrentChargeTime = 0.f;
	bCurrentActionStopsMovement = false;
    if (InputWidget)
    {
        InputWidget->HideCharge();
    }
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Apply a Movement Config to the Outfielder.
///		@param pConfig : 
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::ApplyMovementConfig(UOutfielderMovementConfig* pConfig)
{
    check(pConfig);

    // Set the Max Speed of the Character Movement.
    GetCharacterMovement()->MaxWalkSpeed = pConfig->MaxSpeed;
    MaxPassLeadDistance = pConfig->MaxPassLeadDistance;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : The Pressure value describes "under threat" an Outfielder is based on the
///             opposition. Higher values means higher pressure.
//----------------------------------------------------------------------------------------------------
float AOutfieldCharacter::GetPressure() const
{
    return Pressure;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : This describes how threatening this Outfielder is with regard to scoring a goal. On defense,
///             high threatening players should be marked or pressed. On offense the team should try to
///             get the ball to the highest threatening player.
//----------------------------------------------------------------------------------------------------
float AOutfieldCharacter::GetThreat() const
{
    return Threat;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Calculate the Pressure value for this Outfielder.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::CalculatePressure()
{
    // TODO: The Influence at the outfielder's position should be calculated on in the
    // Field State calculation. I need to pass in a radius around the Outfielder's position that
    // is proportional to the Outfielder's size...
    const auto& fieldState = TeamManager->GetFieldState();
    const auto& influenceMap = fieldState.GetInfluenceMap();

    // TODO: This should be a parameter to tune.
    // The Radius is the only tune value, and the rest should be built in the
    // Outfielder Snapshot.
    FInfluenceArcQueryParams params;
    params.ArcDegrees = 360.f;
    params.Radius = 50.f;
    params.NumberOfPoints = 10;
    params.SpacingMethod = EPointOnCircleSpacingMethod::ByNumberOfPoints;

    float averageInfluence;
    influenceMap.CalcAverageInfluenceInArc(GetActorLocation(), GetActorForwardVector(), params, averageInfluence);

    const float influenceValue = GetInfluenceValue();
    const float influenceDelta = FMath::Abs(influenceValue - averageInfluence);

    Pressure = FMath::Clamp(influenceDelta / FMath::Abs(influenceValue), 0.f, 1.f);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      TODO:
//		
///		@brief : Calculate the Threat value for this Outfielder.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::CalculateThreat()
{
    // If our team is in possession:
    if (TeamManager->GetPossessionState() == ETeamPossessionState::InPossession)
    {
        // TODO: Calculate the Shooting Score:

    }

    else
    {
	    
    }

}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Set the Stat Values for the Debug Stats Widget.
//----------------------------------------------------------------------------------------------------
void AOutfieldCharacter::UpdateStatsWidget() const
{
    if (!StatsWidget)
    {
        return;
    }

    StatsWidget->SetPressure(Pressure);
    StatsWidget->SetThreat(Threat);
}

void AOutfieldCharacter::ShowStats(const bool bEnabled)
{
    if (DebugStatsWidgetComponent)
    {
        DebugStatsWidgetComponent->SetVisibility(bEnabled);
    }
}
