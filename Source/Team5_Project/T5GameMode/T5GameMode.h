#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/DataTable.h" // 데이터 테이블 필수 헤더
#include "T5GameMode.generated.h"

// [규칙서 양식] 엑셀 데이터 구조 정의
USTRUCT(BlueprintType)
struct FCharacterStatRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDamage; // 기본 공격력

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHP;      // 최대 체력
};

UCLASS()
class TEAM5_PROJECT_API AT5GameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AT5GameMode();
    virtual void PostLogin(APlayerController* NewPlayer) override;

    // 게임 진행 관련
    void StartCountdown();
    void GameTimerTick();
    void FinishGame(bool bHunterWin);

    // 판정 및 처형
    // 공격 판정: 룰북을 보고 데미지 명령을 내림
    void ProcessAttack(AController* Attacker, AActor* VictimActor);
    
    // 사망 신고 접수: 캐릭터가 중개해서 호출함
    UFUNCTION(BlueprintCallable)
    void ProcessActorDeath(AActor* Victim, AController* Killer);

protected:
    void OnCountdownTick();
    void RealStartMatch();

    // [룰북] 에디터에서 지정할 데이터 테이블
    UPROPERTY(EditDefaultsOnly, Category = "GameRule")
    UDataTable* StatDataTable;

    // 현재 술래 컨트롤러 저장
    UPROPERTY()
    APlayerController* CurrentHunterPC;

private:
    bool bIsGameStarted;
    int32 CurrentCountdown;
    FTimerHandle TimerHandle_LobbyWait; 
    FTimerHandle TimerHandle_Countdown; 
};