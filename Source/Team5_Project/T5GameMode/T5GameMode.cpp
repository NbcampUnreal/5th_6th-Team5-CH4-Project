#include "T5GameMode.h"
#include "T5GameState.h"
// #include "PlayerCharacter/T5PlayerController.h"
// #include "PlayerCharacter/T5PlayerState.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h" 
#include "GameFramework/PlayerState.h"

AT5GameMode::AT5GameMode()
{
    GameStateClass = AT5GameState::StaticClass();
    
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL) DefaultPawnClass = PlayerPawnBPClass.Class;

    bIsGameStarted = false;
    CurrentHunterPC = nullptr;
}

void AT5GameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    
    if (bIsGameStarted) return;

    if (APlayerState* PS = NewPlayer->GetPlayerState<APlayerState>())
    {
        UE_LOG(LogTemp, Log, TEXT(">>> [접속] %s <<<"), *PS->GetPlayerName());
    }

    // 1명 이상이면 카운트다운 시작
    if (GetNumPlayers() >= 2) 
    {
        GetWorldTimerManager().ClearTimer(TimerHandle_LobbyWait);
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
        UE_LOG(LogTemp, Warning, TEXT("게임 시작 %d초 전..."), CurrentCountdown);
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
    AT5GameState* GS = GetGameState<AT5GameState>();
    if (!GS) return;

    GS->CurrentMatchState = EMatchState::Playing;
    GS->RemainingTime = 60; // 60초 제한시간

    // 1. 모든 플레이어 수집
    TArray<APlayerController*> AllPlayers;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = Cast<APlayerController>(It->Get()))
        {
            AllPlayers.Add(PC);
        }
    }

    if (AllPlayers.Num() == 0) return;

    // 2. [랜덤] 술래 추첨
    int32 HunterIndex = FMath::RandRange(0, AllPlayers.Num() - 1);
    CurrentHunterPC = AllPlayers[HunterIndex]; // 술래 저장

    // 로그 출력
    if (APlayerState* PS = CurrentHunterPC->GetPlayerState<APlayerState>())
    {
        UE_LOG(LogTemp, Error, TEXT(">>> [랜덤 배정] 술래는 '%s' 님 입니다! <<<"), *PS->GetPlayerName());
    }

    // 3. 도망자 수 계산 (전체 인원 - 1)
    GS->SurvivorCount = FMath::Max(0, AllPlayers.Num() - 1); 
    UE_LOG(LogTemp, Warning, TEXT("목표: 도망자 %d명을 잡으세요! (제한시간 60초)"), GS->SurvivorCount);

    // 타이머 시작
    FTimerHandle GameTimerHandle;
    GetWorldTimerManager().SetTimer(GameTimerHandle, this, &AT5GameMode::GameTimerTick, 1.0f, true);
}

void AT5GameMode::GameTimerTick()
{
    AT5GameState* GS = GetGameState<AT5GameState>();
    if (!GS || GS->CurrentMatchState != EMatchState::Playing) return;

    GS->RemainingTime--;

    if (GS->RemainingTime <= 0)
    {
        // 시간 종료 -> 도망자 승리
        FinishGame(false); 
    }
}

// 공격 판정
void AT5GameMode::ProcessAttack(AController* Attacker, AActor* VictimActor)
{
    APlayerController* AttackerPC = Cast<APlayerController>(Attacker);
    if (!AttackerPC) return;

    // 때린 사람이 술래가 아니면 무효 (도망자는 공격 불가)
    if (AttackerPC != CurrentHunterPC)
    {
        return; 
    }

    // 1. 허공 공격 (패널티)
    if (VictimActor == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[패널티] 술래가 허공을 갈랐습니다! HP -10"));
        if (APawn* AttackerPawn = AttackerPC->GetPawn())
        {
             UGameplayStatics::ApplyDamage(AttackerPawn, 10.0f, AttackerPC, AttackerPawn, UDamageType::StaticClass());
        }
        return;
    }

    // 2. 유효타 판정
    APawn* VictimPawn = Cast<APawn>(VictimActor);
    if (!VictimPawn) return;

    AController* VictimController = VictimPawn->GetController();
    
    // "플레이어이면서 && 술래가 아닌 사람" = 진짜 도망자
    bool bIsRealFugitive = (VictimController && VictimController->IsPlayerController() && VictimController != CurrentHunterPC);

    if (bIsRealFugitive)
    {
        // [적중] 진짜 도망자
        UE_LOG(LogTemp, Display, TEXT("[적중] 도망자를 잡았습니다! (도망자 HP -50)"));
        UGameplayStatics::ApplyDamage(VictimActor, 50.0f, AttackerPC, AttackerPC->GetPawn(), UDamageType::StaticClass());
    }
    else
    {
        // [함정] AI (가짜)
        UE_LOG(LogTemp, Error, TEXT("[함정] AI였습니다! (술래 HP -30)"));
        
        // 술래 패널티
        if (APawn* AttackerPawn = AttackerPC->GetPawn())
        {
            UGameplayStatics::ApplyDamage(AttackerPawn, 30.0f, AttackerPC, AttackerPawn, UDamageType::StaticClass());
        }
        
        // AI 제거 (즉사)
        UGameplayStatics::ApplyDamage(VictimActor, 100.0f, AttackerPC, AttackerPC->GetPawn(), UDamageType::StaticClass());
    }
}

// 사망 보고 처리
void AT5GameMode::ProcessActorDeath(AActor* Victim, AController* Killer)
{
    FString VictimName = Victim ? Victim->GetName() : TEXT("Unknown");

    APawn* VictimPawn = Cast<APawn>(Victim);
    
    // 죽은 게 진짜 플레이어(도망자)인지 확인
    if (VictimPawn && VictimPawn->GetController() && VictimPawn->GetController()->IsPlayerController())
    {
        UE_LOG(LogTemp, Warning, TEXT("[검거] 도망자(%s)가 쓰러졌습니다!"), *VictimName);

        AT5GameState* GS = GetGameState<AT5GameState>();
        if (GS)
        {
            GS->SurvivorCount--; 
            if (GS->SurvivorCount <= 0)
            {
                FinishGame(true); // 술래 승리
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[시스템] AI 교란용 캐릭터가 제거되었습니다."));
    }
}

void AT5GameMode::FinishGame(bool bHunterWin)
{
    AT5GameState* GS = GetGameState<AT5GameState>();
    if (GS) GS->CurrentMatchState = EMatchState::GameOver;

    if (bHunterWin)
    {
        UE_LOG(LogTemp, Warning, TEXT(">>> [게임 종료] 술래 승리! 모든 도망자를 잡았습니다! <<<"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT(">>> [게임 종료] 도망자 승리! 시간 종료! <<<"));
    }
}