// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "OutfielderId.generated.h"

USTRUCT(BlueprintType)
struct PROJECTSOCCER_API FOutfielderId
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere)
    FGameplayTag TeamTag = FGameplayTag::EmptyTag;

    /// The Index of the Outfielder in the TeamManager's array.
    UPROPERTY(VisibleAnywhere)
    int32 OutfielderIndex = -1;

    bool operator==(const FOutfielderId& other) const
    {
	    return TeamTag == other.TeamTag && OutfielderIndex == other.OutfielderIndex;
    }

    bool operator!=(const FOutfielderId& other) const
    {
	    return !(*this == other);
    }

    bool IsValid() const
    {
        return TeamTag.IsValid() && OutfielderIndex >= 0;
    }

    friend FORCEINLINE uint32 GetTypeHash(const FOutfielderId& Id)
    {
        return GetTypeHash(Id.TeamTag) ^ GetTypeHash(Id.OutfielderIndex);
    }
};