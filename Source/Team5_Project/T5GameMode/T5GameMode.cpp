#include "T5GameMode.h"
#include "T5PlayerController.h"
#include "T5GameState.h"
#include "T5PlayerState.h"
#include "T5HUD.h"
#include "EngineUtils.h"

AT5GameMode::AT5GameMode()
{
    GameStateClass = AT5GameState::StaticClass();
    PlayerStateClass = AT5PlayerState::StaticClass();
    PlayerControllerClass = AT5PlayerController::StaticClass();
    HUDClass = AT5HUD::StaticClass();
    
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL) DefaultPawnClass = PlayerPawnBPClass.Class;

    bIsGameStarted = false;
}

void AT5GameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    
    if (bIsGameStarted)
    {
        if (AT5PlayerController* PC = Cast<AT5PlayerController>(NewPlayer))
            PC->Client_PrivateMessage(TEXT("게임 진행 중입니다."), FColor::Red, 100);
        return;
    }

    // 접속 알림 (모두에게)
    if (AT5PlayerState* NewPS = NewPlayer->GetPlayerState<AT5PlayerState>())
    {
        FString JoinMsg = FString::Printf(TEXT(">>> %s 님 접속! <<<"), *NewPS->GetPlayerName());
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            if (AT5PlayerController* PC = Cast<AT5PlayerController>(It->Get()))
                PC->Client_PrivateMessage(JoinMsg, FColor::Cyan, -1); // -1: 로그처럼 쌓이게
        }
    }

    // 누군가 들어올 때마다 타이머 리셋
    // "마지막 사람이 들어오고 나서 2초 뒤"에 카운트다운 시작
    GetWorldTimerManager().ClearTimer(TimerHandle_LobbyWait);
    
    // 현재 인원 파악
    int32 NumPlayers = GetNumPlayers();
    if (NumPlayers >= 2) // 최소 2명은 있어야 게임 가능
    {
        FString WaitMsg = FString::Printf(TEXT("현재 %d명. 잠시 후 시작합니다..."), NumPlayers);
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
             if (AT5PlayerController* PC = Cast<AT5PlayerController>(It->Get()))
                PC->Client_PrivateMessage(WaitMsg, FColor::Yellow, 100);
        }

        // 2초 대기 후 StartCountdown 호출
        GetWorldTimerManager().SetTimer(TimerHandle_LobbyWait, this, &AT5GameMode::StartCountdown, 2.0f, false);
    }
}

void AT5GameMode::StartCountdown()
{
    if (bIsGameStarted) return;
    bIsGameStarted = true;
    CurrentCountdown = 3; // 3초 카운트

    GetWorldTimerManager().SetTimer(TimerHandle_Countdown, this, &AT5GameMode::OnCountdownTick, 1.0f, true);
}

void AT5GameMode::OnCountdownTick()
{
    if (CurrentCountdown > 0)
    {
        FString CountMsg = FString::Printf(TEXT("게임 시작 %d초 전..."), CurrentCountdown);
        // 모두에게 카운트다운 전송
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            if (AT5PlayerController* PC = Cast<AT5PlayerController>(It->Get()))
                PC->Client_PrivateMessage(CountMsg, FColor::Yellow, 100);
        }
        CurrentCountdown--;
    }
    else
    {
        GetWorldTimerManager().ClearTimer(TimerHandle_Countdown);
        RealStartMatch();
    }
}

void AT5GameMode::RealStartMatch()
{
    // 모든 플레이어 수집
    TArray<AT5PlayerController*> AllPlayers;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (AT5PlayerController* PC = Cast<AT5PlayerController>(It->Get()))
            AllPlayers.Add(PC);
    }

    if (AllPlayers.Num() < 2) 
    {
        bIsGameStarted = false; // 인원 부족으로 취소
        return; 
    }

    // 랜덤 술래 선정
    int32 HunterIndex = FMath::RandRange(0, AllPlayers.Num() - 1);

    // 역할 분배 및 통보
    for (int32 i = 0; i < AllPlayers.Num(); ++i)
    {
        AT5PlayerController* PC = AllPlayers[i];
        AT5PlayerState* PS = PC->GetPlayerState<AT5PlayerState>();

        if (!PS) continue;

        if (i == HunterIndex)
        {
            PS->SetPlayerRole(EPlayerRole::Hunter);
            PC->Client_SetRole(EPlayerRole::Hunter); // 술래 통보
        }
        else
        {
            PS->SetPlayerRole(EPlayerRole::Animal);
            PC->Client_SetRole(EPlayerRole::Animal); // 도망자 통보
        }
    }
}

void AT5GameMode::ProcessAttack(AController* Attacker, AActor* VictimActor)
{
    AT5PlayerController* AttackerPC = Cast<AT5PlayerController>(Attacker);
    if (!AttackerPC || !VictimActor) return;

    AT5PlayerState* AttackerPS = AttackerPC->GetPlayerState<AT5PlayerState>();
    APawn* VictimPawn = Cast<APawn>(VictimActor);
    AT5PlayerController* VictimPC = VictimPawn ? Cast<AT5PlayerController>(VictimPawn->GetController()) : nullptr;
    AT5PlayerState* VictimPS = VictimPC ? VictimPC->GetPlayerState<AT5PlayerState>() : nullptr;

    if (!AttackerPS || AttackerPS->CurrentRole != EPlayerRole::Hunter)
    {
        AttackerPC->Client_PrivateMessage(TEXT("술래만 공격할 수 있습니다!"), FColor::Red, 100);
        return;
    }

    if (VictimPS)
    {
        AttackerPS->AddScore(1.0f);
        AttackerPC->Client_PrivateMessage(TEXT("[검거 성공!]"), FColor::Green, 100);
        if (VictimPC) VictimPC->Client_PrivateMessage(TEXT("[사망] 술래에게 잡혔습니다!"), FColor::Red, 100);
    }
}