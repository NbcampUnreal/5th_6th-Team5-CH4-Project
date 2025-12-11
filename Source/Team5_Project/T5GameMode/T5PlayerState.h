// T5PlayerState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "T5PlayerState.generated.h"

/**
 * 
 */

// 역할 구분
UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
	None,
	Hunter,  // 술래
	Animal   // 도망자
};


UCLASS()
class TEAM5_PROJECT_API AT5PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
    
	AT5PlayerState();

	// 리플리케이션
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 역할 변수 선언 (서버 -> 클라 동기화)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameData")
	EPlayerRole CurrentRole;
};
