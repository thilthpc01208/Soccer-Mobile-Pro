// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Formation.generated.h"

class UTeamData;

UENUM(BlueprintType)
enum EFormationType
{
    kNeutral    UMETA(DisplayName = "Neutral"),
    kScoredOn   UMETA(DisplayName = "Scored On"),
	kAttack     UMETA(DisplayName = "Defense"),
	kDefense    UMETA(DisplayName = "Attack"),
};

USTRUCT(BlueprintType)
struct FFormationPoint
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere)
    FRotator Rotation = FRotator::ZeroRotator;
};

//----------------------------------------------------------------------------------------------------
///		@brief : A Series of Points that define a positions for a Formation. The Points should be designed
///             where the First point is the most important for that Formation and less important as you go.
//----------------------------------------------------------------------------------------------------
UCLASS()
class PROJECTSOCCER_API UFormation : public UDataAsset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
	TArray<FVector> Points;
    
public:
    UFormation();

    UFUNCTION(BlueprintCallable)
    const TArray<FVector>& GetPoints() const { return Points; }

    size_t GetPointCount() const { return Points.Num(); }
};

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Data Asset FormationData for each Type, Neutral, Scored On, Attack, Defense.
//----------------------------------------------------------------------------------------------------
UCLASS()
class PROJECTSOCCER_API UFormationSet : public UDataAsset
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
    TObjectPtr<UFormation> Neutral = nullptr;

    UPROPERTY(EditAnywhere)
    TObjectPtr<UFormation> ScoredOn = nullptr;

    UPROPERTY(EditAnywhere)
    TObjectPtr<UFormation> Attack = nullptr;

    UPROPERTY(EditAnywhere)
    TObjectPtr<UFormation> Defense = nullptr;
};

//----------------------------------------------------------------------------------------------------
///		@brief : Runtime Formation for a Team.
//----------------------------------------------------------------------------------------------------
UCLASS(BlueprintType)
class PROJECTSOCCER_API UGameFormation : public UObject
{
	GENERATED_BODY()

    static inline FColor s_unassignedColor = FColor::Black;

    UPROPERTY(VisibleAnywhere)
    TArray<FVector> Points;

    UPROPERTY(VisibleAnywhere)
    TArray<bool> PointAssignedStatuses;

    /// Rotation applied to the entire Formation. Used to have the same formation work for both
    /// sides of the field.
    UPROPERTY(VisibleAnywhere)
    FRotator FormationRotation;

    UPROPERTY(VisibleAnywhere)
    FColor ActiveColor;

	UPROPERTY(VisibleAnywhere)
    FColor InactiveColor;

public:
    UGameFormation() = default;

    void SetFormationData(const TObjectPtr<UFormation> pFormation);
    FVector GetLocationOfPoint(const size_t positionIndex) const;
    FVector GetClosestPointTo(const FVector worldLocation) const;

    void SetFormationRotation(const FRotator rotation);
    FRotator GetFormationRotation() const { return FormationRotation; }

    void SetPositionAssigned(const size_t positionIndex, const bool bIsAssigned);
    bool GetPositionAssigned(const size_t positionIndex) const;
    void ClearAllAssignments();

    UFUNCTION(BlueprintPure)
    TArray<FVector> GetPositions() const { return Points; }
    TArray<FVector>& GetMutablePositions() { return Points; }
	const TArray<FVector>& GetPointsConst() const { return Points; };
    size_t GetPointCount() const { return Points.Num(); }

// TODO: Wrap in debug macro...
    void SetDebugColors(const FColor activeColor, const FColor inactiveColor);
    FColor GetDebugColor(const size_t positionIndex) const;

private:
    void ClearFormationData();
};

//----------------------------------------------------------------------------------------------------
///		@brief : Runtime Set of Formations for each EFormationType.
//----------------------------------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FGameFormationSet
{
    GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
    TObjectPtr<UGameFormation> Neutral = nullptr;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UGameFormation> ScoredOn = nullptr;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UGameFormation> Attack = nullptr;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UGameFormation> Defense = nullptr;
};
