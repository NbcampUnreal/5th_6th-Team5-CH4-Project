#include "T5GameMode.h"
#include "T5GameState.h"
#include "PlayerCharacter/T5PlayerController.h"
#include "PlayerCharacter/T5PlayerState.h"

#include "EngineUtils.h"
#include "AI/Team5_DamageTakenComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h" 

AT5GameMode::AT5GameMode()
{
    GameStateClass = AT5GameState::StaticClass();
    PlayerControllerClass = AT5PlayerController::StaticClass();
    PlayerStateClass = AT5PlayerState::StaticClass();
    
    // 기본 캐릭터 블루프린트 연결 (필요시 경로 수정)
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL) DefaultPawnClass = PlayerPawnBPClass.Class;

    bIsGameStarted = false;
    CurrentHunterPC = nullptr;
}

// ---------------------------------------------------------
// 1. 입장 및 게임 준비
// ---------------------------------------------------------
void AT5GameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    
    if (bIsGameStarted) return;

    if (APlayerState* PS = NewPlayer->GetPlayerState<APlayerState>())
    {
        UE_LOG(LogTemp, Log, TEXT(">>> [접속] %s <<<"), *PS->GetPlayerName());
    }

    // [테스트 팁] 혼자 테스트하려면 아래 숫자를 1로 바꾸세요. (평소엔 2)
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

// ---------------------------------------------------------
// 2. 게임 시작 (역할 분배)
// ---------------------------------------------------------
void AT5GameMode::RealStartMatch()
{
    AT5GameState* GS = GetGameState<AT5GameState>();
    if (!GS) return;

    GS->CurrentMatchState = EMatchState::Playing;
    GS->RemainingTime = 60; // 60초 제한

    // 1. 플레이어 수집
    TArray<AT5PlayerController*> AllPlayers;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (AT5PlayerController* PC = Cast<AT5PlayerController>(It->Get()))
        {
            AllPlayers.Add(PC);
        }
    }

    if (AllPlayers.Num() == 0) return;

    // 2. 랜덤 술래 추첨
    int32 HunterIndex = FMath::RandRange(0, AllPlayers.Num() - 1);
    CurrentHunterPC = AllPlayers[HunterIndex];

    // 3. 역할 부여
    for (int32 i = 0; i < AllPlayers.Num(); ++i)
    {
        AT5PlayerController* PC = AllPlayers[i];
        AT5PlayerState* PS = PC->GetPlayerState<AT5PlayerState>(); 

        if (!PS) continue;

        if (i == HunterIndex)
        {
            // [술래] ID 설정 ("Hunter")
            PS->SetPlayerRole(EPlayerRole::Hunter);
            PC->Client_SetRole(EPlayerRole::Hunter); 
            PS->CharacterID = FName("Hunter"); // 룰북 검색용 ID
            
            UE_LOG(LogTemp, Error, TEXT(">>> [술래] %s <<<"), *PS->GetPlayerName());
        }
        else
        {
            // [도망자] ID 설정 ("Animal")
            PS->SetPlayerRole(EPlayerRole::Animal);
            PC->Client_SetRole(EPlayerRole::Animal);
            PS->CharacterID = FName("Animal"); // 룰북 검색용 ID
        }
    }

    GS->SurvivorCount = FMath::Max(0, AllPlayers.Num() - 1); 
    UE_LOG(LogTemp, Warning, TEXT("목표: 도망자 %d명을 잡으세요!"), GS->SurvivorCount);

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
        FinishGame(false); // 시간 종료 (도망자 승)
    }
}

// ---------------------------------------------------------
// 3. 공격 처리 (버그 수정 완료된 버전)
// ---------------------------------------------------------
void AT5GameMode::ProcessAttack(AController* Attacker, AActor* VictimActor)
{
    APlayerController* AttackerPC = Cast<APlayerController>(Attacker);
    if (!AttackerPC || AttackerPC != CurrentHunterPC) return; // 술래만 공격 가능

    // 공격자(술래)의 HP 컴포넌트 가져오기 (패널티 적용 후 HP 확인용)
    APawn* AttackerPawn = AttackerPC->GetPawn();
    UTeam5_DamageTakenComponent* HunterHPComp = AttackerPawn ? AttackerPawn->FindComponentByClass<UTeam5_DamageTakenComponent>() : nullptr;
    
    // -------------------------------------------------------
    // [1단계] 주인이 누구인지 찾기 (무기나 장식품을 때렸을 때 대비)
    // -------------------------------------------------------
    AActor* FinalVictim = VictimActor;
    APawn* VictimPawn = Cast<APawn>(FinalVictim);

    int32 LoopGuard = 0;
    while (FinalVictim && !VictimPawn && LoopGuard < 5)
    {
        AActor* Parent = FinalVictim->GetOwner();
        if (Parent)
        {
            FinalVictim = Parent;
            VictimPawn = Cast<APawn>(FinalVictim);
        }
        else break;
        LoopGuard++;
    }

    // -------------------------------------------------------
    // [2단계] 허공/벽 판정 (Pawn을 못 찾음) -> 술래 패널티
    // -------------------------------------------------------
    if (!VictimPawn)
    {
         if (AttackerPawn)
        {
            // 술래에게 데미지 적용
            UGameplayStatics::ApplyDamage(AttackerPawn, 10.0f, AttackerPC, AttackerPawn, UDamageType::StaticClass());

            // 술래 현재 HP 로그 출력
            if (HunterHPComp)
            {
                float CurrentHP = HunterHPComp->GetCurrentHP();
                UE_LOG(LogTemp, Warning, TEXT("    -> [허공/벽] 술래 패널티 HP: %.1f"), CurrentHP);

                if (CurrentHP <= 0.0f)
                {
                    UE_LOG(LogTemp, Error, TEXT(">>> [결과] 술래가 패널티 누적으로 사망(패배)했습니다! <<<"));
                }
            }
        }
        return;
    }

    // -------------------------------------------------------
    // [3단계] 공격력 조회 (룰북)
    // -------------------------------------------------------
    float DamageToApply = 50.0f; // 기본값
    if (AT5PlayerState* AttackerPS = AttackerPC->GetPlayerState<AT5PlayerState>())
    {
        if (StatDataTable)
        {
            static const FString ContextString(TEXT("StatLookUp"));
            FCharacterStatRow* Row = StatDataTable->FindRow<FCharacterStatRow>(AttackerPS->CharacterID, ContextString);
            if (Row) DamageToApply = Row->BaseDamage; 
        }
    }

    // -------------------------------------------------------
    // [4단계] 대상별 피해 적용 (중복 로직 삭제됨)
    // -------------------------------------------------------
    AController* VictimController = VictimPawn->GetController();
        
    // (A) AI(함정) 타격
    if (VictimController && !VictimController->IsPlayerController())
    {
        UE_LOG(LogTemp, Error, TEXT(">>> [함정] AI(%s) 타격! (술래 -30)"), *VictimActor->GetName());
    
        // 1. 술래 패널티
        UGameplayStatics::ApplyDamage(AttackerPawn, 30.0f, AttackerPC, nullptr, UDamageType::StaticClass());
        
        // 2. AI 제거 (즉사)
        UGameplayStatics::ApplyDamage(VictimActor, 9999.0f, AttackerPC, nullptr, UDamageType::StaticClass());

        // 술래 사망 체크
        if (HunterHPComp && HunterHPComp->GetCurrentHP() <= 0.0f) 
            UE_LOG(LogTemp, Error, TEXT(">>> 술래 사망 (패배) <<<"));
    }
    // (B) 도망자(플레이어) 타격
    else if (VictimController && VictimController->IsPlayerController())
    {
        UE_LOG(LogTemp, Display, TEXT(">>> [적중] 도망자(%s) 타격! (데미지 %.1f)"), *VictimActor->GetName(), DamageToApply);
        
        // 도망자에게 데미지
        UGameplayStatics::ApplyDamage(VictimActor, DamageToApply, AttackerPC, nullptr, UDamageType::StaticClass());
    }
    // (C) 컨트롤러 없는 Pawn (단순 배치된 봇 등)
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[주의] 컨트롤러 없는 Pawn(%s) 타격"), *VictimActor->GetName());
        UGameplayStatics::ApplyDamage(VictimActor, DamageToApply, AttackerPC, nullptr, UDamageType::StaticClass());
    }
}

// ---------------------------------------------------------
// 4. 사망 신고 접수 (캐릭터 -> 게임모드)
// ---------------------------------------------------------
void AT5GameMode::ProcessActorDeath(AActor* Victim, AController* Killer)
{
    FString VictimName = Victim ? Victim->GetName() : TEXT("Unknown");
    APawn* VictimPawn = Cast<APawn>(Victim);
    
    // 진짜 도망자(플레이어)가 죽었을 때만 카운트 감소
    if (VictimPawn && VictimPawn->GetController() && VictimPawn->GetController()->IsPlayerController())
    {
        UE_LOG(LogTemp, Warning, TEXT("[검거] 도망자(%s) 탈락!"), *VictimName);

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
        UE_LOG(LogTemp, Log, TEXT("[시스템] AI가 제거되었습니다."));
    }
}

// ---------------------------------------------------------
// 5. 게임 종료 처리
// ---------------------------------------------------------
void AT5GameMode::FinishGame(bool bHunterWin)
{
    AT5GameState* GS = GetGameState<AT5GameState>();
    if (!GS) return;

    // 이미 종료되었으면 무시
    if (GS->CurrentMatchState == EMatchState::GameOver) return;

    GS->CurrentMatchState = EMatchState::GameOver;
    GS->WinningTeam = bHunterWin ? EWinningTeam::Hunter : EWinningTeam::Animal;

    UE_LOG(LogTemp, Warning, TEXT(">>> [종료] %s 승리! <<<"), bHunterWin ? TEXT("술래") : TEXT("도망자"));

    // 모든 플레이어에게 종료 알림 (멈춰!)
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (AT5PlayerController* PC = Cast<AT5PlayerController>(It->Get()))
        {
            PC->Client_OnGameOver(GS->WinningTeam);
        }
    }
}