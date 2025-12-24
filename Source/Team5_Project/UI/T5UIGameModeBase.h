// Source/Team5_Project/UI/T5UIGameModeBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T5UIGameModeBase.generated.h"

UCLASS()
class TEAM5_PROJECT_API AT5UIGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AT5UIGameModeBase(); // 생성자 추가

	// [여기가 핵심!] 이 줄이 있어야 블루프린트에서 검색이 됩니다.
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void StartGameSequence();
};