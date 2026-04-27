// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/Entities/Field/GoalTrigger.h"

#include "ProjectSoccer/Entities/SoccerBall.h"
#include "Components/BoxComponent.h"
#include "ProjectSoccer/GameMode/ProjectSoccerGameMode.h"

// Sets default values
AGoalTrigger::AGoalTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>("Trigger");
    SetRootComponent(Box);
    Box->SetCollisionResponseToAllChannels(ECR_Ignore);
    Box->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
    //Box->OnComponentBeginOverlap.AddDynamic(this, &AGoalTrigger::OnBeginOverlap);
    Box->OnComponentEndOverlap.AddDynamic(this, &AGoalTrigger::OnEndOverlap);
}

void AGoalTrigger::BeginPlay()
{
	Super::BeginPlay();

    AProjectSoccerGameMode::OnMatchInit().AddUObject(this, &AGoalTrigger::OnMatchInit);
    AProjectSoccerGameMode::OnMatchPhaseBegin().AddUObject(this, &AGoalTrigger::OnMatchPhaseBegin);

    // Calculate our post positions:
    FVector current = GetActorLocation();
    current.Z = 0.f;
    FRotator rotation = GetActorRotation();

    PostPositions.SetNumZeroed(2, EAllowShrinking::Yes);
    const FVector extent = Box->GetScaledBoxExtent();
    const FVector right = rotation.RotateVector(FVector(0, extent.Y, 0));

    PostPositions[0] = current - right;
    PostPositions[1] = current + right;

    FromLeftPostToRight = (PostPositions[1] - PostPositions[0]);

    CalculateShotTestLocations();
}

void AGoalTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    AProjectSoccerGameMode::OnMatchInit().RemoveAll(this);
    AProjectSoccerGameMode::OnMatchPhaseBegin().RemoveAll(this);
}

void AGoalTrigger::OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode)
{
    GameMode = pGameMode;
}

void AGoalTrigger::OnMatchPhaseBegin(const EMatchPhase phase)
{
	switch (phase)
	{
		case EMatchPhase::Kickoff:
		{
            Box->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
			break;
		}

		default: break;
	}
}

void AGoalTrigger::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Block the ball, making the goal trap it.
	if (ASoccerBall* pBall = Cast<ASoccerBall>(OtherActor))
    {
        // Check to see if the ball is behind the collider:
        const FVector ballLocation = pBall->GetActorLocation();
        const FVector toBall = ballLocation - GetActorLocation();

        const float dot = FVector::DotProduct(toBall, GetActorForwardVector());
        if (dot < 0.f)
        {
            const auto& contactInfo = pBall->GetLastContactInfo();

            FGoalScoredInfo info
        	{
                contactInfo.Location,
				ScoringTeamTag,
                contactInfo.Outfielder
            };

			Box->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);

            // TODO: It would be nice to pass in more info about this.
            GameMode->GoalScored(info);
        }
    }
}

float AGoalTrigger::GetGoalWidth() const
{
    return Box->GetScaledBoxExtent().Y * 2.f;
}

float AGoalTrigger::GetGoalHeight() const
{
    return Box->GetScaledBoxExtent().Z * 2.f;
}

void AGoalTrigger::CalculateShotTestLocations()
{
    const float boxWidth = GetGoalWidth() / ShotTestGridResolution.X;
    const float boxHeight = GetGoalHeight() / ShotTestGridResolution.Y;

    const float halfBoxWidth = GetGoalWidth() * 0.5f;
    const float halfBoxHeight = GetGoalHeight() * 0.5f;

    // This is the easiest position on the goal to score from.
    const FVector bottomCenter = GetLeftPost() + (FromLeftPostToRight * 0.5f);
    const FVector right = FromLeftPostToRight.GetSafeNormal();
    const FVector up = FVector::UpVector;

    // Shot Location are at the Center of each tile on the Box Grid.
    const FVector toCenterOfBox = (right * halfBoxWidth) + (up * halfBoxHeight);
    const FVector rightOffset = right * boxWidth;
    const FVector upOffset = up * boxHeight;

    const FVector topLeft = GetLeftPost() + (FVector::UpVector * GetGoalHeight());

    // The maximum distance from the easiest position to the hardest position (top corner);
    const float maxDistanceSquared = FVector::DistSquared(bottomCenter, topLeft);

    FVector currentPosition = GetLeftPost();

    for (int32 y = 0; y < ShotTestGridResolution.Y; ++y)
    {
        currentPosition = GetLeftPost() + (upOffset * y);

        for (int32 x = 0; x < ShotTestGridResolution.X; ++x)
        {
            const FVector location = currentPosition + toCenterOfBox;
            const float difficulty = FVector::DistSquared(bottomCenter, location) / maxDistanceSquared;
            ShotTestLocations.Emplace(location, difficulty);

            currentPosition += rightOffset;
        }
    }
}
