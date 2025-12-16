// T5GameState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "T5GameState.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	Waiting,
	Playing,
	GameOver
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
};
