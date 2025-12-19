#include "T5PlayerController.h"
#include "T5GameMode/T5GameMode.h"
#include "T5PlayerState.h"
#include "GameFramework/Character.h" // 추가
#include "GameFramework/CharacterMovementComponent.h" // 추가
#include "Kismet/GameplayStatics.h" // 추가
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

/*void AT5PlayerController::Client_OnGameOver_Implementation(EWinningTeam Winner)
{
    // 1. 캐릭터 움직임 정지
    if (APawn* MyPawn = GetPawn())
    {
        // 입력 차단
        MyPawn->DisableInput(this);

        // 이동 컴포넌트 강제 정지
        if (ACharacter* MyChar = Cast<ACharacter>(MyPawn))
        {
            if (MyChar->GetCharacterMovement())
            {
                MyChar->GetCharacterMovement()->StopMovementImmediately();
                MyChar->GetCharacterMovement()->DisableMovement();
            }
        }
    }

    // 2. 마우스 커서 보이게 하기 (UI 조작용)
    bShowMouseCursor = true;
    SetInputMode(FInputModeUIOnly());

    // 3. 결과 로그 출력
    FString WinStr = (Winner == EWinningTeam::Hunter) ? TEXT("술래 승리!") : TEXT("도망자 승리!");
    UE_LOG(LogTemp, Warning, TEXT(">>> [게임 종료] %s <<<"), *WinStr);

    // [참고] 나중에 여기서 위젯(WBP_Result)을 띄우는 코드를 추가하면 됩니다.
}*/