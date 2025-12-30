#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "T5GameState.generated.h"

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	Waiting,
	Playing,
	GameOver
};

// [필수 추가] 승리 팀을 구분하기 위한 열거형
UENUM(BlueprintType)
enum class EWinningTeam : uint8
{
	None,
	Hunter,
	Animal,
	Draw
};

UCLASS()
class TEAM5_PROJECT_API AT5GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AT5GameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameData")
	EMatchState CurrentMatchState;
    
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameData")
	int32 RemainingTime;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameData")
	int32 SurvivorCount;

	// [필수 추가] 누가 이겼는지 모든 클라이언트가 알아야 함
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameData")
	EWinningTeam WinningTeam;
};