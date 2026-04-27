// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SoccerGameState.generated.h"

class AOutfieldCharacter;

UENUM()
enum ETeamType
{
	kTeamA,
    kTeamB
};

UCLASS()
class PROJECTSOCCER_API ASoccerGameState : public AGameStateBase
{
	GENERATED_BODY()

    UPROPERTY()
    TArray<AOutfieldCharacter*> TeamAOutfielders;

    UPROPERTY()
    TArray<AOutfieldCharacter*> TeamBOutfielders;

public:
    void AddOutfielder(AOutfieldCharacter* pOutfielder, ETeamType team);
    AOutfieldCharacter* FindClosestTeammate(FVector worldPosition, ETeamType team);
};
