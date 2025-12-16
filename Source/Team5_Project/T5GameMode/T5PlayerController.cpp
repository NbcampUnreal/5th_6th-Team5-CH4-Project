#include "T5PlayerController.h"
#include "T5GameMode.h"
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

    // 내 캐릭터(Pawn) 가져오기
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    /* [디버그] 역할 배정 시 로그 및 화면 출력 준비 로직 (비활성화)
    int32 MyId = PS->GetPlayerId();
    FString RoleMsg;
    FColor MsgColor;

    if (NewRole == EPlayerRole::Hunter)
    {
        RoleMsg = FString::Printf(TEXT("ID:%d\n[술래]\nHP:%.0f"), MyId, PS->CurrentHP);
        MsgColor = FColor::Red;
    }
    else
    {
        RoleMsg = FString::Printf(TEXT("ID:%d\n[도망자]\nHP:%.0f"), MyId, PS->CurrentHP);
        MsgColor = FColor::Green;
    }
    
    if (IsLocalController())
    {
        UE_LOG(LogTemp, Log, TEXT("내 역할이 %d로 설정되었습니다."), (int32)NewRole);
    }
    */
}

// 공지사항(접속, 카운트다운) 출력용
void AT5PlayerController::Client_PrivateMessage_Implementation(const FString& Message, FColor Color, int32 MsgKey)
{
    if (!IsLocalController()) return;
    
    // [디버그] GEngine 화면 메시지 출력 비활성화
    // if (GEngine) GEngine->AddOnScreenDebugMessage(MsgKey, 5.0f, Color, Message);
}

// [테스트 코드] 치트 구현부 비활성화
/*
void AT5PlayerController::CheatHit() { Server_CheatHit(); }
void AT5PlayerController::Server_CheatHit_Implementation()
{
    AT5GameMode* GM = Cast<AT5GameMode>(GetWorld()->GetAuthGameMode());
    if (GM)
    {
        AActor* Victim = nullptr;
        for (TActorIterator<APawn> It(GetWorld()); It; ++It)
        {
            if (*It != GetPawn()) { Victim = *It; break; }
        }
        if (Victim) GM->ProcessAttack(this, Victim);
    }
}
*/