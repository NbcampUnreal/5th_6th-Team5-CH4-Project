// T5GameMode.cpp

#include "T5GameMode.h"
#include "T5PlayerState.h"
#include "T5GameState.h"
#include "Kismet/GameplayStatics.h"

AT5GameMode::AT5GameMode()
{
    // T5PlayerState를 기본 클래스로 지정
    PlayerStateClass = AT5PlayerState::StaticClass();
    GameStateClass = AT5GameState::StaticClass();
}

void AT5GameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // 접속한 플레이어의 PlayerState
    AT5PlayerState* PS = NewPlayer->GetPlayerState<AT5PlayerState>();
    
    if (PS)
    {
        // 'Animal(도망자)'로 기본 설정
        // 게임 시작 시 랜덤으로 한 명만 Hunter로 변경
        PS->CurrentRole = EPlayerRole::Animal;
        UE_LOG(LogTemp, Warning, TEXT("플레이어 입장: %s, 역할 설정 완료: Animal"), *NewPlayer->GetName());
    }
    
    // [테스트 코드] 접속자가 2명이 되면 바로 게임 시작!
    if (GetNumPlayers() >= 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("2명이 모였습니다. 게임을 시작합니다!"));
        StartGameMatch();
    }
}

void AT5GameMode::StartGameMatch()
{
    // 현재 접속한 모든 플레이어(PlayerState) 로드
    if (!GameState) return;

    TArray<APlayerState*> AllPlayers = GameState->PlayerArray;

    if (AllPlayers.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("접속자가 없어서 게임을 시작할 수 없습니다."));
        return;
    }

    // 랜덤 인덱스 뽑기 (0 ~ 플레이어 수 - 1)
    int32 RandomIndex = FMath::RandRange(0, AllPlayers.Num() - 1);

    // 루프를 돌면서 역할 분배
    for (int32 i = 0; i < AllPlayers.Num(); i++)
    {
        // 형변환, T5PlayerState로
        AT5PlayerState* PS = Cast<AT5PlayerState>(AllPlayers[i]);
        if (PS)
        {
            if (i == RandomIndex)
            {
                // 술래(Hunter) 선정
                PS->CurrentRole = EPlayerRole::Hunter;
                UE_LOG(LogTemp, Warning, TEXT("[당첨] 술래가 선정되었습니다: %s"), *PS->GetPlayerName());
            }
            else
            {
                // 나머지는 동물(Animal)
                PS->CurrentRole = EPlayerRole::Animal;
            }
        }
    }
}

