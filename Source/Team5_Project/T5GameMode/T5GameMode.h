// T5GameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T5GameMode.generated.h"

/**
 * 
 */
UCLASS()
class TEAM5_PROJECT_API AT5GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// 생성자 선언
	AT5GameMode();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 게임 시작 시 술래 정하기
    UFUNCTION(BlueprintCallable, Category = "GameLogic")
    void StartGameMatch();
	
};
