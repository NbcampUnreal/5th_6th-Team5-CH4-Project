#include "T5PlayerController.h"
#include "T5GameMode/T5GameMode.h"
#include "T5PlayerState.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

void AT5PlayerController::Client_SetRole_Implementation(EPlayerRole NewRole)
{
    if (!IsLocalController()) return;

    AT5PlayerState* PS = GetPlayerState<AT5PlayerState>();
    if (PS == nullptr || PS->GetPlayerId() == -1)
    {
        FTimerHandle WaitTimer;
        GetWorld()->GetTimerManager().SetTimer(
            WaitTimer,
            [this, NewRole]() { Client_SetRole(NewRole); },
            0.1f, false
        );
        return;
    }

    BP_OnRoleAssigned(NewRole);
}

void AT5PlayerController::Client_PrivateMessage_Implementation(const FString& Msg, FColor Color, int32 Key)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(Key, 2.0f, Color, Msg);
    }
}

void AT5PlayerController::Client_OnGameOver_Implementation(EWinningTeam Winner)
{
    if (APawn* MyPawn = GetPawn())
    {
        MyPawn->DisableInput(this);

        if (ACharacter* MyChar = Cast<ACharacter>(MyPawn))
        {
            if (MyChar->GetCharacterMovement())
            {
                MyChar->GetCharacterMovement()->StopMovementImmediately();
                MyChar->GetCharacterMovement()->DisableMovement();
            }
        }
    }

    bShowMouseCursor = true;
    SetInputMode(FInputModeUIOnly());

    FString WinStr = (Winner == EWinningTeam::Hunter) ? TEXT("술래 승리!") : TEXT("도망자 승리!");
    UE_LOG(LogTemp, Warning, TEXT(">>> [게임 종료] %s <<<"), *WinStr);
}

void AT5PlayerController::Client_ShowDieUI_Implementation()
{
    if (!IsLocalController()) return;

    if (DieWidgetInstance) return;

    if (!DieWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Client_ShowDieUI] DieWidgetClass is null. Set it in BP_T5PlayerController Defaults."));
        return;
    }

    DieWidgetInstance = CreateWidget<UUserWidget>(this, DieWidgetClass);
    if (!DieWidgetInstance) return;

    DieWidgetInstance->AddToViewport(200);

    bShowMouseCursor = true;

    FInputModeUIOnly Mode;
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(Mode);
}
