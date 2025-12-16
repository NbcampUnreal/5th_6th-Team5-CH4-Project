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
	void ProcessAttack(AController* Attacker, AActor* VictimActor);
	
	UFUNCTION(BlueprintCallable)
	void ProcessActorDeath(AActor* Victim, AController* Killer);

protected:
	void OnCountdownTick();
	void RealStartMatch();

private:
	bool bIsGameStarted;
	int32 CurrentCountdown;
	FTimerHandle TimerHandle_LobbyWait; // 접속 대기 타이머
	FTimerHandle TimerHandle_Countdown; // 3,2,1 카운트다운 타이머
};