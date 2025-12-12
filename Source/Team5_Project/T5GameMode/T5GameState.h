// T5GameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "T5GameState.generated.h"

/**
 * 
 */

// 게임 진행 상태 (대기 -> 플레이 -> 결과)
UENUM(BlueprintType)
enum class EMatchState : uint8
{
	Waiting,	// 로비 대기
	Playing,	// 게임 시작 (플레이)
	GameOver	// 게임 종료 (결과)
};

UCLASS()
class TEAM5_PROJECT_API AT5GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AT5GameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// 현재 매치 상태
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameData")
	EMatchState CurrentMatchState;

	// 남은 시간 (초 단위)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameData")
	int32 RemainingTime;
};
