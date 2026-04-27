// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ProjectSoccer/GameMode/MatchEntityInterface.h"
#include "ProjectSoccer/Entities/Outfielder/OutfielderId.h"
#include "OutfieldPlayerController.generated.h"

class AOutfieldAIController;
class ATeamManager;
struct FInputActionValue;
class UInputMappingContext;
class UInputAction;
class AOutfieldCharacter;

DECLARE_EVENT_TwoParams(AOutfieldPlayerController, FPlayerPossessed, AOutfieldCharacter* /*pCharacterReleased*/, AOutfieldCharacter* /*pCharacterPossessed*/)

UCLASS()
class PROJECTSOCCER_API AOutfieldPlayerController : public APlayerController, public IMatchEntityInterface 
{
	GENERATED_BODY()

    /** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* PassAction = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* ChargePassAction = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* SwitchPlayerAction = nullptr;

     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* ChargeShotOrTackle = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* ShootOrTackleAction = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* PauseAction = nullptr;

    UPROPERTY(VisibleAnywhere, Category= OutfieldPlayer)
    FOutfielderId StartPlayerId;

    UPROPERTY(VisibleAnywhere, Category= OutfieldPlayer)
    FOutfielderId ControlledPlayerId;

    UPROPERTY(VisibleAnywhere, Category= OutfieldPlayer)
    TObjectPtr<class AOutfieldCharacter> ControlledCharacter;
    
    UPROPERTY()
    TObjectPtr<AProjectSoccerGameMode> GameMode;

    UPROPERTY()
    TObjectPtr<ATeamManager> TeamManager;

    UPROPERTY()
    AOutfieldAIController* LastAIController;

    FVector2D MovementVector;
	FPlayerPossessed PlayerPossessedEvent;

public:
    AOutfieldPlayerController();

    virtual void OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode) override;
    virtual void OnMatchPhaseBegin(const EMatchPhase phase) override;

    void SetTeamManager(TObjectPtr<ATeamManager> pTeamManager);
    void SetStartOutfielder(const FOutfielderId id);
    void SetControlledOutfielder(const FOutfielderId id);
    void RepossessStartOutfielder();
    AOutfieldCharacter* GetControlledOutfielder() const { return ControlledCharacter; }
    AOutfieldAIController* GetOverwrittenAIController() const { return LastAIController; }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void SetupInputComponent() override;

	UFUNCTION() void Move(const FInputActionValue& Value);
    UFUNCTION() void Pass(const FInputActionValue& Value);
    UFUNCTION() void SwitchPlayer(const FInputActionValue& Value);
    UFUNCTION() void ShootOrTackle(const FInputActionValue& Value);
    UFUNCTION() void BeginChargePass(const FInputActionValue& Value);
    UFUNCTION() void BeginChargeShot(const FInputActionValue& Value);
    UFUNCTION() void TogglePause(const FInputActionValue& Value);

    void OnBallPossessionChange(AOutfieldCharacter* pCharacter);
    virtual void OnPossess(APawn* InPawn) override;
};
