#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "T5HUD.generated.h"

UCLASS()
class TEAM5_PROJECT_API AT5HUD : public AHUD
{
	GENERATED_BODY()

public:
	// 매 프레임마다 화면에 그림을 그리는 함수
	virtual void DrawHUD() override;
};