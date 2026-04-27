// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ProjectSoccer/GameMode/MatchEntityInterface.h"
#include "GoalTrigger.generated.h"

struct FShotTestLocation
{
	FVector ShotLocation; // The Location where the Shot will pass through the Goal Plane.
    float Difficulty;     // The Difficulty of the Shot, 0 being the easiest, 1 being the hardest.
};

UCLASS()
class PROJECTSOCCER_API AGoalTrigger : public AActor, public IMatchEntityInterface
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere)
	class UBoxComponent* Box;

	/*The Tag representing the team that gets a point when scoring on this Goal.*/
	UPROPERTY(EditAnywhere, Category = SoccerGame)
    FGameplayTag ScoringTeamTag;

    UPROPERTY()
    TObjectPtr<AProjectSoccerGameMode> GameMode;

    UPROPERTY(EditDefaultsOnly, Category = SoccerGame)
    FIntPoint ShotTestGridResolution = FIntPoint(4, 3);

    TArray<FShotTestLocation> ShotTestLocations;
    TArray<FVector> PostPositions;
    FVector FromLeftPostToRight;

public:	
	// Sets default values for this actor's properties
	AGoalTrigger();

    void SetScoringTeamTag(const FGameplayTag& tag) { ScoringTeamTag = tag; }
    FGameplayTag GetScoringTeamTag() const { return ScoringTeamTag; }

	//----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the two post positions, [0] being the left post, [1] being the right post.
	//----------------------------------------------------------------------------------------------------
	const TArray<FVector>& GetPostPositions() const { return PostPositions; }
	const TArray<FShotTestLocation>& GetShotTestLocations() const { return ShotTestLocations; }
    const FVector& GetLeftPost() const { return PostPositions[0]; }
    const FVector& GetVectorFromLeftPostToRight() const { return FromLeftPostToRight; }
    float GetGoalWidth() const;
    float GetGoalHeight() const;

private:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void OnMatchInit(const TObjectPtr<AProjectSoccerGameMode>& pGameMode) override;
    virtual void OnMatchPhaseBegin(const EMatchPhase phase) override;

    UFUNCTION()
    void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void CalculateShotTestLocations();
};
