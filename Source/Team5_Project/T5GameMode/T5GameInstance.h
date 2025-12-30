#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "T5GameInstance.generated.h"

UCLASS()
class TEAM5_PROJECT_API UT5GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UT5GameInstance();

	// [이게 빠져 있었습니다!] 인원수를 저장할 변수 선언
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 TargetPlayerCount;
};