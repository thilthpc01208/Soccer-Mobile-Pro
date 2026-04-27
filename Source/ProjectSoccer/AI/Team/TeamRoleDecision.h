// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamRoleScoreProvider.h"
#include "Engine/DataAsset.h"
#include "ProjectSoccer/Entities/Team/TeamRoles.h"
#include "ProjectSoccer/AI/Utility/Consideration.h"
#include "TeamRoleDecision.generated.h"

class AOutfieldAIController;
class UTeamRoleConsideration;

DECLARE_LOG_CATEGORY_EXTERN(LogTeamRoleDecision, Log, All);

struct FScoredRole
{
	ERoleType RoleType;
    float Score;
};

// TODO: Create an Asset Menu for this
UCLASS()
class PROJECTSOCCER_API UTeamRoleDecision : public UDataAsset
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = Role)
    ERoleType RoleType;

    UPROPERTY(EditAnywhere, Category = Role, meta = (AllowedClasses = "/Script/ProjectSoccer.UTeamRoleScoreProvider"))
    TArray<TObjectPtr<UConsideration>> RoleConsiderations;

    UPROPERTY()
    TArray<float> Scores;

public:
    virtual FScoredRole GetScore(const FTeamRoleContext& context);

private:
    float CombineScores();
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
