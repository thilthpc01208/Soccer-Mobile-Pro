// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectSoccer/Commands/Command.h"
#include "CoreMinimal.h"
#include "CommandBuffer.generated.h"

UCLASS()
class PROJECTSOCCER_API UCommandBuffer : public UObject
{
    GENERATED_BODY()

    // Buffer of commands:
    TArray<UCommand> Buffer;

    // Command history (Ring Buffer):
    /*TArray<UCommand> History;
    size_t HistoryHead = 0;
    size_t HistoryTail = 0;*/

public:
	UCommandBuffer();

    void ProcessBuffer();
	void ClearBuffer();
    void PushCommand(const UCommand& command);
};
