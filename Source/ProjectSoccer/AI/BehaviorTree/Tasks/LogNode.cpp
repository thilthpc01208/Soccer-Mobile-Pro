// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/BehaviorTree/Tasks/LogNode.h"

// NOTE: I have this because here seems to be an issue when using the ENUM_CLASS_FLAGS macro for bitmasks in the editor.
// https://forums.unrealengine.com/t/c-bitmask-enums-appear-to-be-offset-by-1/370610/8
static bool IsLogTypeSet(const int32 value, const ELogType testType)
{
    return (value & (1 << static_cast<uint32_t>(testType))) > 0;
}

EBTNodeResult::Type ULogNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (IsLogTypeSet(LogType, ELogType::Console))
    {
		UE_LOG(LogBehaviorTree, Display, TEXT("%s"), *Message);
    }

    if (IsLogTypeSet(LogType, ELogType::Viewport))
    {
		GEngine->AddOnScreenDebugMessage(MessageKey, DisplayTime, MessageColor, FString::Printf(TEXT("%s"), *Message));
    }
    
	return EBTNodeResult::Succeeded;
}