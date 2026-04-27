// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectSoccer/GameMode/MatchEntityInterface.h"
#include "GameCamera.generated.h"

class ASoccerBall;

UCLASS()
class PROJECTSOCCER_API AGameCamera : public AActor, public IMatchEntityInterface
{
	GENERATED_BODY()

    /** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

    UPROPERTY(EditAnywhere, Category = GameCamera)
    AActor* LeftTarget;

    UPROPERTY(EditAnywhere, Category = GameCamera)
    AActor* RightTarget;

    UPROPERTY(EditAnywhere, Category = GameCamera, meta = (ClampMin = 50.f))
    float CameraSpeed = 400.f;

    UPROPERTY()
    AActor* FollowTarget;

    FVector LeftLimit;
    FVector RightLimit;

public:	

	AGameCamera();

    FRotator GetCameraRotation() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode) override;
    virtual void OnMatchPhaseBegin(const EMatchPhase phase) override;
    void SetCameraFollowTarget(AActor* pTarget) { FollowTarget = pTarget; };
};
