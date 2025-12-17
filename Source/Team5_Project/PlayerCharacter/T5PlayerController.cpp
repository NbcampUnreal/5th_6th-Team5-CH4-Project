#include "T5PlayerController.h"
#include "T5GameMode/T5GameMode.h"
#include "T5PlayerState.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"


void AT5PlayerController::Client_SetRole_Implementation(EPlayerRole NewRole)
{
    if (!IsLocalController()) return;

    AT5PlayerState* PS = GetPlayerState<AT5PlayerState>();
    if (PS == nullptr || PS->GetPlayerId() == -1)
    {
        FTimerHandle WaitTimer;
        GetWorld()->GetTimerManager().SetTimer(WaitTimer, [this, NewRole]() { Client_SetRole(NewRole); }, 0.1f, false);
        return; 
    }

}

void AT5PlayerController::Client_PrivateMessage_Implementation(const FString& Msg, FColor Color, int32 Key)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(Key, 2.0f, Color, Msg);
    }
}