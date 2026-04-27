// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/Entities/GameCamera.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ProjectSoccer/Entities/SoccerBall.h"
#include "ProjectSoccer/GameMode/ProjectSoccerGameMode.h"

// Sets default values
AGameCamera::AGameCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

    // Create a camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(false); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

    // Create a camera
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

// Called when the game starts or when spawned
void AGameCamera::BeginPlay()
{
	Super::BeginPlay();
    AProjectSoccerGameMode::OnMatchInit().AddUObject(this, &AGameCamera::OnMatchInit);
    AProjectSoccerGameMode::OnMatchPhaseBegin().AddUObject(this, &AGameCamera::OnMatchPhaseBegin);

    if (LeftTarget)
        LeftLimit = LeftTarget->GetActorLocation();

    if (RightTarget)
        RightLimit = RightTarget->GetActorLocation();
}

void AGameCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    AProjectSoccerGameMode::OnMatchInit().RemoveAll(this);
    AProjectSoccerGameMode::OnMatchPhaseBegin().RemoveAll(this);
}

// Called every frame
void AGameCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FollowTarget)
	{
		const FVector currentLocation = GetActorLocation();
		FVector targetLocation = currentLocation;
        targetLocation.Y = FMath::Clamp(FollowTarget->GetActorLocation().Y, LeftLimit.Y, RightLimit.Y);

        const float step = FMath::Clamp(CameraSpeed * DeltaTime, 0.f, 1.f);
        const FVector lerped = FMath::Lerp(currentLocation, targetLocation, step);
        SetActorLocation(lerped);
	}
}

void AGameCamera::OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode)
{
    // Set up the Camera Component
    UCameraComponent* pCamera = GetComponentByClass<UCameraComponent>();
    check(pCamera);

    // Set the Follow Target to the Ball
    FollowTarget = Cast<AActor>(pGameMode->GetSoccerBall());

    pCamera->SetActive(true);
}

void AGameCamera::OnMatchPhaseBegin(const EMatchPhase phase)
{
	// TODO:
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Get the world Rotation of the Camera.
//----------------------------------------------------------------------------------------------------
FRotator AGameCamera::GetCameraRotation() const
{
	auto rotation = GetActorRotation();
    rotation -= CameraBoom->GetRelativeRotation();
    return rotation;
}

