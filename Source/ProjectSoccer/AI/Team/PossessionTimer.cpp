// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/AI/Team/PossessionTimer.h"

#include "ProjectSoccer/GameMode/ProjectSoccerGameMode.h"
#include "ProjectSoccer/Entities/SoccerBall.h"

DEFINE_LOG_CATEGORY(LogPossessionTimer)

void APossessionTimer::BeginPlay()
{
    Super::BeginPlay();

    AProjectSoccerGameMode::OnMatchInit().AddUObject(this, &APossessionTimer::OnMatchInit);
    AProjectSoccerGameMode::OnMatchPhaseBegin().AddUObject(this, &APossessionTimer::OnMatchPhaseBegin);
    AProjectSoccerGameMode::OnMatchEvent().AddUObject(this, &APossessionTimer::OnMatchEvent);
}

void APossessionTimer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    AProjectSoccerGameMode::OnMatchInit().RemoveAll(this);
    AProjectSoccerGameMode::OnMatchPhaseBegin().RemoveAll(this);
    AProjectSoccerGameMode::OnMatchEvent().RemoveAll(this);
}

void APossessionTimer::OnMatchPhaseBegin(const EMatchPhase phase)
{
	switch (phase)
	{
		case EMatchPhase::Kickoff:
		case EMatchPhase::PostMatch:
        {
            StopTimer();
            break;
        }
        
		default: break;
	}
}

void APossessionTimer::OnMatchEvent(const EMatchEvent event, const FMatchState& state)
{
    switch (event)
    {
	    case EMatchEvent::OutfielderPossessionChange:
	    {
	        OnPossessionChanged(state.GetTeamInPossessionTag());
	        break;
	    }

	    default: break;
    }
}

void APossessionTimer::StartTimer()
{
	StopTimer();
    GetWorldTimerManager().SetTimer(TimerHandle, this, &APossessionTimer::UpdateTime, kPossessionTickInterval, true);
}

void APossessionTimer::StopTimer()
{
	if (TimerHandle.IsValid())
        GetWorldTimerManager().ClearTimer(TimerHandle);

    PossessionTimeElapsed = 0.f;
}

void APossessionTimer::ResetTimer()
{
    StopTimer();
    StartTimer();
}

float APossessionTimer::GetTimeElapsed() const
{
    return PossessionTimeElapsed;
}

void APossessionTimer::OnPossessionChanged(FGameplayTag tag)
{
    // If no team has possession, then stop the timer.
	if (tag == FGameplayTag::EmptyTag)
		StopTimer();

    // Otherwise, restart it.
    else
	    ResetTimer();
}

void APossessionTimer::UpdateTime()
{
	PossessionTimeElapsed += kPossessionTickInterval;	
}
