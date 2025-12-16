#include "T5GameMode.h"
#include "T5PlayerController.h"
#include "T5GameState.h"
#include "T5PlayerState.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AT5GameMode::AT5GameMode()
{
    GameStateClass = AT5GameState::StaticClass();
    PlayerStateClass = AT5PlayerState::StaticClass();
    PlayerControllerClass = AT5PlayerController::StaticClass();
    
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL) DefaultPawnClass = PlayerPawnBPClass.Class;

    bIsGameStarted = false;
}

void AT5GameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    
    if (bIsGameStarted)
    {
        /* [테스트 코드] 게임 중 난입 차단 메시지
        if (AT5PlayerController* PC = Cast<AT5PlayerController>(NewPlayer))
            PC->Client_PrivateMessage(TEXT("게임 진행 중입니다."), FColor::Red, 100);
        */
        return;
    }

    // [디버그] 접속 알림 (모두에게)
    /*
    if (AT5PlayerState* NewPS = NewPlayer->GetPlayerState<AT5PlayerState>())
    {
        FString JoinMsg = FString::Printf(TEXT(">>> %s 님 접속! <<<"), *NewPS->GetPlayerName());
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            if (AT5PlayerController* PC = Cast<AT5PlayerController>(It->Get()))
                PC->Client_PrivateMessage(JoinMsg, FColor::Cyan, -1);
        }
    }
    */

    // 타이머 리셋 및 카운트다운 준비
    GetWorldTimerManager().ClearTimer(TimerHandle_LobbyWait);
    
    int32 NumPlayers = GetNumPlayers();
    if (NumPlayers >= 2) 
    {
        /* [디버그] 대기 메시지
        FString WaitMsg = FString::Printf(TEXT("현재 %d명. 잠시 후 시작합니다..."), NumPlayers);
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
             if (AT5PlayerController* PC = Cast<AT5PlayerController>(It->Get()))
                PC->Client_PrivateMessage(WaitMsg, FColor::Yellow, 100);
        }
        */

        GetWorldTimerManager().SetTimer(TimerHandle_LobbyWait, this, &AT5GameMode::StartCountdown, 2.0f, false);
    }
}

void AT5GameMode::StartCountdown()
{
    if (bIsGameStarted) return;
    bIsGameStarted = true;
    CurrentCountdown = 3; 

    GetWorldTimerManager().SetTimer(TimerHandle_Countdown, this, &AT5GameMode::OnCountdownTick, 1.0f, true);
}

void AT5GameMode::OnCountdownTick()
{
    if (CurrentCountdown > 0)
    {
        /* [디버그] 카운트다운 출력
        FString CountMsg = FString::Printf(TEXT("게임 시작 %d초 전..."), CurrentCountdown);
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            if (AT5PlayerController* PC = Cast<AT5PlayerController>(It->Get()))
                PC->Client_PrivateMessage(CountMsg, FColor::Yellow, 100);
        }
        */
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
    // [테스트 코드] 자동 역할 분배 (추후 로직 복구 시 주석 해제)
    /*
    TArray<AT5PlayerController*> AllPlayers;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (AT5PlayerController* PC = Cast<AT5PlayerController>(It->Get()))
            AllPlayers.Add(PC);
    }

    if (AllPlayers.Num() < 2) 
    {
        bIsGameStarted = false; 
        return; 
    }

    int32 HunterIndex = FMath::RandRange(0, AllPlayers.Num() - 1);

    for (int32 i = 0; i < AllPlayers.Num(); ++i)
    {
        AT5PlayerController* PC = AllPlayers[i];
        AT5PlayerState* PS = PC->GetPlayerState<AT5PlayerState>();

        if (!PS) continue;

        if (i == HunterIndex)
        {
            PS->SetPlayerRole(EPlayerRole::Hunter);
            PC->Client_SetRole(EPlayerRole::Hunter);
        }
        else
        {
            PS->SetPlayerRole(EPlayerRole::Animal);
            PC->Client_SetRole(EPlayerRole::Animal);
        }
    }
    */
}

// 공격 판정 및 결과 처리
void AT5GameMode::ProcessAttack(AController* Attacker, AActor* VictimActor)
{
    // 1. 공격자 확인 (플레이어여야 함)
    AT5PlayerController* AttackerPC = Cast<AT5PlayerController>(Attacker);
    if (!AttackerPC) return;

    AT5PlayerState* AttackerPS = AttackerPC->GetPlayerState<AT5PlayerState>();
    if (!AttackerPS) return;

    // ----------------------------------------------------------------
    // 상황 1: 허공을 공격함 (Miss)
    // ----------------------------------------------------------------
    if (VictimActor == nullptr)
    {
        // 술래가 헛스윙하면 체력 10 감소
        if (AttackerPS->GetPlayerRole() == EPlayerRole::Hunter)
        {
            AttackerPS->ApplyDamage(10.0f);
        }
        return;
    }

    // ----------------------------------------------------------------
    // 상황 2: 다른 [플레이어]를 공격함
    // ----------------------------------------------------------------
    if (APawn* VictimPawn = Cast<APawn>(VictimActor))
    {
        // Pawn -> Controller -> PlayerState 순으로 가져옴
        AT5PlayerController* VictimPC = Cast<AT5PlayerController>(VictimPawn->GetController());
        
        if (VictimPC) // 상대방이 플레이어
        {
            AT5PlayerState* VictimPS = VictimPC->GetPlayerState<AT5PlayerState>();
            if (VictimPS)
            {
                // 술래가 도망자를 공격 (검거)
                if (AttackerPS->GetPlayerRole() == EPlayerRole::Hunter && 
                    VictimPS->GetPlayerRole() == EPlayerRole::Animal)
                {
                    VictimPS->ApplyDamage(50.0f);
                }
                
            }
            return;
        }
    }

    // ----------------------------------------------------------------
    // 상황 3: [AI]를 공격함 (플레이어가 아님)
    // ----------------------------------------------------------------
    // 플레이어가 아닌데 공격받았다면 AI로 간주
    
    // 술래가 AI를 공격 -> AI 파괴
    if (AttackerPS->GetPlayerRole() == EPlayerRole::Hunter)
    {
        UGameplayStatics::ApplyDamage(
            VictimActor, 
            100.0f,
            AttackerPC,
            AttackerPC->GetPawn(),
            UDamageType::StaticClass()
        );

    }
}

void AT5GameMode::ProcessActorDeath(AActor* Victim, AController* Killer)
{
    FString KillerName = Killer ? Killer->GetName() : TEXT("Unknown");
    FString VictimName = Victim ? Victim->GetName() : TEXT("Unknown");

    FString KillMsg = FString::Printf(TEXT("[System] %s 님이 %s 을(를) 처치했습니다!"), *KillerName, *VictimName);
    
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (AT5PlayerController* PC = Cast<AT5PlayerController>(It->Get()))
        {
            PC->Client_PrivateMessage(KillMsg, FColor::Red, -1);
        }
    }   
    UE_LOG(LogTemp, Warning, TEXT("!!! DEATH EVENT !!! %s killed %s"), *KillerName, *VictimName);
}