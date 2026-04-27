// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ERoleType : uint8
{
	StayInFormation
    , PressPlayer
    , Guard
    , GetOpen
    , InPossession
    , GetBall
};

//USTRUCT(BlueprintType)
//struct PROJECTSOCCER_API FTeamRole
//{
//    GENERATED_BODY()
//
//	UPROPERTY(VisibleAnywhere)
//    ERoleType RoleType;
//
//   
//};
