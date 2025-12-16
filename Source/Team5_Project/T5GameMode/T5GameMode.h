#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T5GameMode.generated.h"

UCLASS()
class TEAM5_PROJECT_API AT5GameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AT5GameMode();
    virtual void PostLogin(APlayerController* NewPlayer) override;

    void StartCountdown();
    
    // 공격 판정 함수
    void ProcessAttack(AController* Attacker, AActor* VictimActor);
    
    // 사망 처리 함수
    UFUNCTION(BlueprintCallable)
    void ProcessActorDeath(AActor* Victim, AController* Killer);

    // 1초마다 흐르는 타이머
    void GameTimerTick();

    // 게임 종료 처리
    void FinishGame(bool bHunterWin);

protected:
    void OnCountdownTick();
    void RealStartMatch();

    // [중요] 현재 술래인 플레이어 컨트롤러를 저장
    UPROPERTY()
    APlayerController* CurrentHunterPC;

private:
    bool bIsGameStarted;
    int32 CurrentCountdown;
    FTimerHandle TimerHandle_LobbyWait; 
    FTimerHandle TimerHandle_Countdown; 
};