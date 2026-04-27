// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LogNode.generated.h"

UENUM(Blueprintable, meta=(Bitflags))
enum ELogType
{
    Viewport,
    Console,
};

UCLASS()
class PROJECTSOCCER_API ULogNode : public UBTTaskNode
{
	GENERATED_BODY()

	UPROPERTY(Category = Log, EditAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/ProjectSoccer.ELogType"))
    int32 LogType;

	UPROPERTY(Category = Log, BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))
    FString Message = "";

    UPROPERTY(Category = "Log|Viewport", BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true", EditConditionHides = "bDisplayOnViewport"))
    FColor MessageColor = FColor::Cyan;

    UPROPERTY(Category =  "Log|Viewport", BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true", EditConditionHides = "bDisplayOnViewport"))
    int32 MessageKey = -1;

    UPROPERTY(Category =  "Log|Viewport", BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true", EditConditionHides = "bDisplayOnViewport"))
    float DisplayTime = 3.f;

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
