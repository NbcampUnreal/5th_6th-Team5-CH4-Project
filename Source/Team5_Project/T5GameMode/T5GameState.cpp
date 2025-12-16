// T5GameState.cpp

#include "T5GameMode/T5GameState.h"
#include "Net/UnrealNetwork.h"

AT5GameState::AT5GameState()
{
    CurrentMatchState = EMatchState::Waiting;
    RemainingTime = 60; 
    SurvivorCount = 0;  
}

void AT5GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AT5GameState, CurrentMatchState);
    DOREPLIFETIME(AT5GameState, RemainingTime);
    DOREPLIFETIME(AT5GameState, SurvivorCount);
}