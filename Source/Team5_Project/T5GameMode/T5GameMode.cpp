#include "T5GameMode.h"
#include "T5GameState.h"
#include "PlayerCharacter/T5PlayerController.h"
#include "PlayerCharacter/T5PlayerState.h"

#include "EngineUtils.h"
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

    // 2명 이상이면 카운트다운 시작
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
// 3. 공격 처리 (Data-Driven 핵심)
// ---------------------------------------------------------
void AT5GameMode::ProcessAttack(AController* Attacker, AActor* VictimActor)
{
    APlayerController* AttackerPC = Cast<APlayerController>(Attacker);
    if (!AttackerPC || AttackerPC != CurrentHunterPC) return; // 술래만 공격 가능

    // A. 허공 공격 (패널티)
    if (VictimActor == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[패널티] 허공 공격! HP -10"));
        if (APawn* AttackerPawn = AttackerPC->GetPawn())
        {
             UGameplayStatics::ApplyDamage(AttackerPawn, 10.0f, AttackerPC, AttackerPawn, UDamageType::StaticClass());
        }
        return;
    }

    // B. 데이터 테이블(룰북)에서 공격력 조회
    float DamageToApply = 50.0f; // 기본값 (테이블 없을 때 대비)

    if (AT5PlayerState* AttackerPS = AttackerPC->GetPlayerState<AT5PlayerState>())
    {
        if (StatDataTable)
        {
            static const FString ContextString(TEXT("StatLookUp"));
            // 플레이어 스테이트에 있는 CharacterID("Hunter")로 검색
            FCharacterStatRow* Row = StatDataTable->FindRow<FCharacterStatRow>(AttackerPS->CharacterID, ContextString);
            
            if (Row)
            {
                DamageToApply = Row->BaseDamage; // 룰북 값 가져오기
            }
        }
    }

    // C. 명령 하달 (ApplyDamage)
    // AI 함정인지, 진짜 도망자인지는 여기서 구분하지 않고 일단 때림
    // 단, AI 함정 로직(술래 패널티)이 필요하다면 여기서 체크
    
    if (APawn* VictimPawn = Cast<APawn>(VictimActor))
    {
        AController* VictimController = VictimPawn->GetController();
        
        // AI(함정) 체크
        if (VictimController && !VictimController->IsPlayerController())
        {
            UE_LOG(LogTemp, Error, TEXT("[함정] AI 공격! (술래 HP -30)"));
            // 술래에게 패널티
            UGameplayStatics::ApplyDamage(AttackerPC->GetPawn(), 30.0f, AttackerPC, nullptr, UDamageType::StaticClass());
            
            // AI는 즉사급 데미지 줘서 제거
            UGameplayStatics::ApplyDamage(VictimActor, 9999.0f, AttackerPC, nullptr, UDamageType::StaticClass());
        }
        else
        {
            // 진짜 도망자 -> 룰북 데미지 적용
            UE_LOG(LogTemp, Display, TEXT("[적중] 도망자 타격! 데미지: %.1f"), DamageToApply);
            UGameplayStatics::ApplyDamage(VictimActor, DamageToApply, AttackerPC, nullptr, UDamageType::StaticClass());
        }
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

void AT5GameMode::FinishGame(bool bHunterWin)
{
    AT5GameState* GS = GetGameState<AT5GameState>();
    if (GS) GS->CurrentMatchState = EMatchState::GameOver;

    if (bHunterWin)
    {
        UE_LOG(LogTemp, Warning, TEXT(">>> [종료] 술래 승리! <<<"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT(">>> [종료] 도망자 승리! (시간 초과) <<<"));
    }
}