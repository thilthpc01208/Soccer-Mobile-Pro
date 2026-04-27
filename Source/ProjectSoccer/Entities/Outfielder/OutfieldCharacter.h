// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "OutfielderMovementConfig.h"
#include "ProjectSoccer/Entities/Team/TeamManager.h"
#include "GameFramework/Character.h"
#include "ProjectSoccer/GameMode/MatchEntityInterface.h"
#include "ProjectSoccer/Entities/Outfielder/OutfielderId.h"
#include "OutfieldCharacter.generated.h"

class AInfluenceMap;
class AOutfieldAIController;
class UTeamData;
class AOutfieldCharacter;
class ATeamManager;
class UBillboardWidgetComponent;

DECLARE_EVENT_OneParam(AOutfieldCharacter, FGainedPossession, AOutfieldCharacter*);
DECLARE_LOG_CATEGORY_EXTERN(LogOutfieldCharacter, Log, All)

//----------------------------------------------------------------------------------------------------
///		@brief : Struct containing all the data needed to set up a new Outfielder.
//----------------------------------------------------------------------------------------------------
struct FOutfielderCreateInfo
{
    TObjectPtr<ATeamManager> TeamManager;
    UMaterialInstance* TeamMaterial;
    FOutfielderId Id;
    bool bPlayerTeam; // TODO: This boolean sucks, and is only applicable to a single player game.
};

//----------------------------------------------------------------------------------------------------
///		@brief : An Outfielder is a controllable character playing in the Field during a Soccer Match.
//----------------------------------------------------------------------------------------------------
UCLASS()
class PROJECTSOCCER_API AOutfieldCharacter : public ACharacter, public IMatchEntityInterface
{
	GENERATED_BODY()

    inline static FGainedPossession OnPlayerGainedPossessionEvent = {};

public:
    static FGainedPossession& OnOutfielderGainedPossessionOfBall() { return OnPlayerGainedPossessionEvent; }

private:
	//----------------------------------------------------------------------------------------------------
	///		@brief : Used to capture the Ball when near the Outfielder's Physical Body.
	//----------------------------------------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere)
    TObjectPtr<class UCapsuleComponent> BallCaptureCapsule;

	//----------------------------------------------------------------------------------------------------
    ///		@brief : Used to capture the Ball near the BallPossessionTarget.
	//----------------------------------------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere)
    TObjectPtr<class USphereComponent> BallCaptureSphere;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class USceneComponent> BallPossessionTarget;

    UPROPERTY(EditAnywhere)
    TObjectPtr<class UBillboardWidgetComponent> PlayerIcon;

    UPROPERTY(EditAnywhere)
    TObjectPtr<class UWidgetComponent> InputWidgetComponent;

//------------------------------
// TODO: Debug Only  
    UPROPERTY(EditAnywhere)
    TObjectPtr<class UBillboardWidgetComponent> DebugStatsWidgetComponent;
//------------------------------

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Influence")
    float InfluenceRadius = 50.f;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Influence")
    float InfluenceScalar = 1.0f;

	UPROPERTY(EditAnywhere, Category = OutfieldCharacter)
    float PossessionCountdownTime = 0.5f;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Mesh")
    FName TeamMaterialSlotName = "TeamMaterial";

    UPROPERTY(VisibleAnywhere, Category = "OutfieldCharacter|GameData")
    TObjectPtr<ATeamManager> TeamManager;
    
    UPROPERTY()
    TObjectPtr<class ASoccerBall> Ball = nullptr;

    UPROPERTY()
    TObjectPtr<class AGameCamera> GameCamera = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "OutfieldCharacter|GameData")
    FOutfielderId OutfielderId;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Movement")
    UOutfielderMovementConfig* BaseConfig;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Charge Behavior")
    UCurveFloat* ChargeCurve;

    UPROPERTY(VisibleAnywhere, Category = "OutfieldCharacter|Charge Behavior")
    float CurrentCharge = 0.f;

    UPROPERTY(VisibleAnywhere, Category = "OutfieldCharacter|Charge Behavior")
    float ChargeMaxTime = 0.5f;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Pass Behavior", meta = (ClampMin = 0.001f))
    float PassMinSpeed = 1600.f;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Pass Behavior", meta = (ClampMin = 0.001f))
    float PassMaxSpeed = 2000.f;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Pass Behavior", meta = (ClampMin = 0.001f))
    float ManualPassDistOnMinCharge = 400.f;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Pass Behavior", meta = (ClampMin = 0.001f))
    float ManualPassDistOnMaxCharge = 1600.f;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Pass Behavior", meta = (ClampMin = 0.001f))
    float PassArcHeight = 1.f;

    //----------------------------------------------------------------------------------------------------
	///		@brief : The angle, in degrees, that the Shot must be aimed within to be considered on target.
	//----------------------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Pass Behavior", meta = (ClampMin = 0.f, ClampMax = 90.f))
    float PassAimAngleThreshold = 40.f;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Pass Behavior")
    UCurveFloat* PassVelocityCurve;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Shot Behavior", meta = (ClampMin = 0.001f))
    float ShotMinSpeed = 1800.f;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Shot Behavior", meta = (ClampMin = 0.001f))
    float ShotMaxSpeed = 2100.f;

    UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Shot Behavior", meta = (ClampMin = 0.001f))
    float ShotArcHeight = 50.f;

	//----------------------------------------------------------------------------------------------------
	///		@brief : The angle, in degrees, that the Shot must be aimed within to be considered on target.
	//----------------------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "OutfieldCharacter|Shot Behavior", meta = (ClampMin = 0.f, ClampMax = 90.f))
    float ShotAimAngleThreshold = 5.f;

    UPROPERTY(VisibleAnywhere, Category = "OutfieldCharacter|GameData")
    bool bCanCaptureBall = true;

    UPROPERTY(VisibleAnywhere, Category = "OutfieldCharacter|GameData")
    bool bOnPlayerTeam = false;

    UPROPERTY()
    class UOutfielderInputWidget* InputWidget = nullptr;

    //------------------------------
	// TODO: Debug Only  
    UPROPERTY()
    class UOutfielderDebugStatsWidget* StatsWidget = nullptr;
    //------------------------------

    FVector DefaultLocalPossessionTargetLocation;
    FVector MovementVector;
    FVector LockedVelocity;
    FRotator InitialInputWidgetRotation;
    FTimerHandle PossessionLockedTimerHandle;
    FTimerHandle MovementLockTimerHandle;
    float MaxPassLeadDistance = 0.f;
    float Pressure = 0.f;
    float Threat = 0.f;
    float CurrentChargeTime = 0.f;
    bool bCurrentActionStopsMovement;
    bool bMovementVectorLocked;
    bool bIsCharging = false;

public:
	AOutfieldCharacter();

    // Blueprint Exposed
    UFUNCTION(BlueprintCallable)
    ATeamManager* GetTeamManager() const { return TeamManager; }

    // Match Entity Interface
    virtual void OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode) override;
    virtual void OnMatchPhaseBegin(const EMatchPhase phase) override;
    
    // Actions
    void Move(FVector2D movementVector);
    void Pass();
    void PassToTeammate(TObjectPtr<AOutfieldCharacter> pTeammate);
    void PassInCurrentDirection();
    void Shoot(const FVector2D movementVector);
    void Tackle(const FVector2D& movementVector);
    void BeginChargeShot();
    void BeginChargePass();
    void RemoveBallPossession();

    // Movement
    void LockMovementForDuration(const float duration);
    void LockMovement();
    void ReleaseMovement();

    // Icon
    void SetControlledByPlayer(const int index) const;
    void SetControlledIconCPU() const;

    // Setters
    void SetOutfielderInfo(const FOutfielderCreateInfo& info);
    void SetInfluenceScalar(const float scalar) { InfluenceScalar = scalar; }

    // Getters
    FVector GetPassTargetLocation() const;
    FORCEINLINE bool HasPossessionOfBall() const { return Ball != nullptr;}
    FORCEINLINE float GetInfluenceValue() const { return TeamManager->GetInfluenceValue() * InfluenceScalar; }
    FORCEINLINE float GetBaseInfluence() const { return TeamManager->GetInfluenceValue(); }
    FORCEINLINE float GetInfluenceRadius() const { return InfluenceRadius; }
    FORCEINLINE const FOutfielderId& GetOutfielderId() const { return OutfielderId; }
    FORCEINLINE FGameplayTag GetTeamTag() const { return OutfielderId.TeamTag; }
    float GetPressure() const;
    float GetThreat() const;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    void BeginCharge();
    void EndCharge();
    void StartPossessionCooldown();
    void FinishPossessionLockTimer();
    void FinishMovementLockTimer();
    void OnReset();
    FVector GetShotLocation(const FVector& shotDirection, const float shotCharge) const;

    UFUNCTION()
    void OnGainedPossession(AOutfieldCharacter* pOutfielder);

    void ApplyMovementConfig(UOutfielderMovementConfig* pConfig);

    void CalculatePressure();
    void CalculateThreat();

    //------------------------------
	// TODO: Debug Only
    UFUNCTION() void ShowStats(const bool bShow);
    void UpdateStatsWidget() const;
    //------------------------------
};
