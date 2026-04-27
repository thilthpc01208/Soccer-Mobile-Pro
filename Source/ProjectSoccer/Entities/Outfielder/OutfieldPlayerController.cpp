// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/Entities/Outfielder/OutfieldPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "OutfieldAIController.h"
#include "OutfieldCharacter.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"
#include "ProjectSoccer/GameMode/ProjectSoccerGameMode.h"
#include "ProjectSoccer/UI/SoccerModeHUD.h"

AOutfieldPlayerController::AOutfieldPlayerController()
	: LastAIController(nullptr)
{
	bAutoManageActiveCameraTarget = false;
}

void AOutfieldPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Register for Events.
    AProjectSoccerGameMode::OnMatchInit().AddUObject(this, &AOutfieldPlayerController::OnMatchInit);
    AProjectSoccerGameMode::OnMatchPhaseBegin().AddUObject(this, &AOutfieldPlayerController::OnMatchPhaseBegin);
    AOutfieldCharacter::OnOutfielderGainedPossessionOfBall().AddUObject(this, &AOutfieldPlayerController::OnBallPossessionChange);

    SetInputMode(FInputModeGameOnly());

    // Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AOutfieldPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // Unregister for Events.
    AProjectSoccerGameMode::OnMatchInit().RemoveAll(this);
    AProjectSoccerGameMode::OnMatchPhaseBegin().RemoveAll(this);
    AOutfieldCharacter::OnOutfielderGainedPossessionOfBall().RemoveAll(this);
}

void AOutfieldPlayerController::OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode)
{
    GameMode = pGameMode;
}

void AOutfieldPlayerController::SetTeamManager(TObjectPtr<ATeamManager> pTeamManager)
{
    TeamManager = pTeamManager;
}

void AOutfieldPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
	    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOutfieldPlayerController::Move);

        // Passing
        EnhancedInputComponent->BindAction(ChargePassAction, ETriggerEvent::Triggered, this, &AOutfieldPlayerController::BeginChargePass);
        EnhancedInputComponent->BindAction(PassAction, ETriggerEvent::Triggered, this, &AOutfieldPlayerController::Pass);

        // Shooting and Tackling
	    EnhancedInputComponent->BindAction(ChargeShotOrTackle, ETriggerEvent::Triggered, this, &AOutfieldPlayerController::BeginChargeShot);
	    EnhancedInputComponent->BindAction(ShootOrTackleAction, ETriggerEvent::Triggered, this, &AOutfieldPlayerController::ShootOrTackle);

        // Switching Players
	    EnhancedInputComponent->BindAction(SwitchPlayerAction, ETriggerEvent::Started, this, &AOutfieldPlayerController::SwitchPlayer);

        // Pausing
        EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Triggered, this, &AOutfieldPlayerController::TogglePause);
    }
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Assign the Starting Outfielder for this PlayerController. When going back to
///            the Kickoff phase, their starting Outfielder will be repossessed.
///		@param id : ID of the Outfielder to start with.
//----------------------------------------------------------------------------------------------------
void AOutfieldPlayerController::SetStartOutfielder(const FOutfielderId id)
{
    StartPlayerId = id;
}

void AOutfieldPlayerController::SetControlledOutfielder(const FOutfielderId id)
{
    check(GameMode);
    ControlledPlayerId = id;

    if (GameMode->GetCurrentMatchPhase() == EMatchPhase::Playing)
    {
	    const auto pPlayer = TeamManager->GetOutfielder(id);
        Possess(pPlayer);
    }
}

void AOutfieldPlayerController::OnMatchPhaseBegin(const EMatchPhase phase)
{
    switch (phase)
    {
        // Un-possess the outfielder when the match is reset or ended.
	    case EMatchPhase::Kickoff:
        case EMatchPhase::PostMatch:
	    {
			ControlledCharacter = nullptr;
			LastAIController = nullptr;
            UnPossess();
		    break;
	    }
            
    	default: break;
    }
}

void AOutfieldPlayerController::RepossessStartOutfielder()
{
	const auto pCharacter = TeamManager->GetOutfielder(StartPlayerId);
	Possess(pCharacter);
}

void AOutfieldPlayerController::Move(const FInputActionValue& Value)
{
    if (!ControlledCharacter)
        return;

    MovementVector = Value.Get<FVector2D>();
    //GEngine->AddOnScreenDebugMessage(50, 3.f, FColor::Green, FString::Printf(TEXT("PlayerInput: %s"), *MovementVector.ToString()));
    ControlledCharacter->Move(MovementVector);
}


//----------------------------------------------------------------------------------------------------
///		@brief : Pass to the closest teammate in the direction of the Movement Vector.
//----------------------------------------------------------------------------------------------------
void AOutfieldPlayerController::Pass(const FInputActionValue& Value)
{
    if (!ControlledCharacter)
        return;

    ControlledCharacter->Pass();
}

//----------------------------------------------------------------------------------------------------
///		@brief : Begin Charging the Pass of the Outfielder.
//----------------------------------------------------------------------------------------------------
void AOutfieldPlayerController::BeginChargePass(const FInputActionValue& Value)
{
    if (!ControlledCharacter)
        return;

    ControlledCharacter->BeginChargePass();
}

//----------------------------------------------------------------------------------------------------
///		@brief : Charge the Shot of the Outfielder.
//----------------------------------------------------------------------------------------------------
void AOutfieldPlayerController::BeginChargeShot(const FInputActionValue& Value)
{
    if (!ControlledCharacter)
        return;

    ControlledCharacter->BeginChargeShot();
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Toggle the Pause Menu. (Right now, it's the DebugMenu...)
///		@param Value : 
//----------------------------------------------------------------------------------------------------
void AOutfieldPlayerController::TogglePause(const FInputActionValue& Value)
{
    auto* pHUD = Cast<ASoccerModeHUD>(GetHUD());
    check(pHUD);
    pHUD->ShowDebugMenu();
}

//----------------------------------------------------------------------------------------------------
///		@brief : Shoot the Ball if the Outfielder is in possession, or Tackle if not.
//----------------------------------------------------------------------------------------------------
void AOutfieldPlayerController::ShootOrTackle(const FInputActionValue& Value)
{
	if (!ControlledCharacter)
        return;

    if (ControlledCharacter->HasPossessionOfBall())
    {
         ControlledCharacter->Shoot(MovementVector);
    }

    else
    {
         ControlledCharacter->Tackle(MovementVector);
    }
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//      - I should probably look into switching to the closest teammate to the Ball.
//
///		@brief : Switch to the closest player to the current player, in the direction of the Movement
///            Vector.
//----------------------------------------------------------------------------------------------------
void AOutfieldPlayerController::SwitchPlayer(const FInputActionValue& Value)
{
    if (!ControlledCharacter)
        return;

    if (ControlledCharacter->HasPossessionOfBall())
    {
        // TODO: 
	    // Consider an option to toggle a dribble mode.
        return;
    }

    const auto pClosestPlayer = TeamManager->GetClosestTeammate(ControlledCharacter->GetOutfielderId());
    if (!pClosestPlayer)
        return;

    UnPossess();
	Possess(pClosestPlayer);
}

void AOutfieldPlayerController::OnPossess(APawn* InPawn)
{
    if (InPawn)
    {
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("Possessed: %s"), *(InPawn->GetHumanReadableName())));
    }

    // Have the LastAIController repossess the left Agent:
    if (LastAIController)
    {
	    LastAIController->Possess(ControlledCharacter);
    }

    // If this pawn was possessed by an AI controller previously, get a reference to it.
    if (AOutfieldAIController* pAiController = Cast<AOutfieldAIController>(InPawn->Controller))
    {
		LastAIController = pAiController;
	}

    Super::OnPossess(InPawn);

	ControlledCharacter = Cast<AOutfieldCharacter>(InPawn);
    if (!ControlledCharacter)
    {
	    UE_LOG(LogOutfieldCharacter, Error, TEXT("Failed to cast Pawn to OutFieldCharacter!"));
    }

    else
    {
        // Set our id.
	    ControlledPlayerId = ControlledCharacter->GetOutfielderId();

        // Hack: This is a temporary solution to set the icon for the controlled player.
        // Right now, there is only 1 player.
        ControlledCharacter->SetControlledByPlayer(0);
    }
}

void AOutfieldPlayerController::OnBallPossessionChange(AOutfieldCharacter* pCharacter)
{
	if (ControlledPlayerId.OutfielderIndex != pCharacter->GetOutfielderId().OutfielderIndex && ControlledPlayerId.TeamTag == pCharacter->GetOutfielderId().TeamTag)
	{
        Possess(pCharacter);
	}
}