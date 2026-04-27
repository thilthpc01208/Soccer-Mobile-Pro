// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/Team/TeamRoleDecision.h"

#include "TeamRoleConsideration.h"

DEFINE_LOG_CATEGORY(LogTeamRoleDecision)

FScoredRole UTeamRoleDecision::GetScore(const FTeamRoleContext& context)
{
    FScoredRole scoredRole { RoleType, 0.f};

    if (RoleConsiderations.IsEmpty())
        return scoredRole;

    /*if (RoleConsiderations.Num() != Scores.Num())
    {
	    Scores.SetNumZeroed(RoleConsiderations.Num(), true);
    }*/

    for (size_t i = 0; i < RoleConsiderations.Num(); ++i)
    {
        if (ITeamRoleScoreProvider* pProvider = Cast<ITeamRoleScoreProvider>(RoleConsiderations[i]))
        {
	        Scores[i] = pProvider->GetScore(context);

	        // If any score is 0.f, fail out.
	        if (Scores[i] == 0.f)
	        {
	            return scoredRole;
	        }
        }

        else
        {
	        return scoredRole;
        }
    }

    scoredRole.Score = CombineScores();
    return scoredRole;
}

float UTeamRoleDecision::CombineScores()
{
	if (Scores.IsEmpty())
        return 0.f;

    float result = 1.f;

    for (size_t i = 0; i < Scores.Num(); ++i)
    {
	    result *= Scores[i];
    }

    return FMath::Pow(result , 1.f / static_cast<float>(Scores.Num()));
}

void UTeamRoleDecision::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName propertyName =  PropertyChangedEvent.GetMemberPropertyName();

    if (propertyName == GET_MEMBER_NAME_CHECKED(UTeamRoleDecision, RoleConsiderations))
    {
	    if (RoleConsiderations.Num() != Scores.Num())
	    {
		    Scores.SetNumZeroed(RoleConsiderations.Num(), EAllowShrinking::Yes);
	    }


        // Check that each implement the interface:
        for (size_t i = 0; i < RoleConsiderations.Num(); ++i)
        {
            if (RoleConsiderations[i] == nullptr)
                continue;

	        if (!RoleConsiderations[i]->GetClass()->ImplementsInterface(UTeamRoleScoreProvider::StaticClass()))
	        {
                UE_LOG(LogTeamRoleDecision, Error, TEXT("Object added to RoleConsiderations does not implement the ITeamRoleScoreProvider interface!"));
				//RoleConsiderations.Swap(i, RoleConsiderations.Num() - 1);
                //RoleConsiderations.RemoveAt(RoleConsiderations.Num() - 1);
	        }
        }
    }
}