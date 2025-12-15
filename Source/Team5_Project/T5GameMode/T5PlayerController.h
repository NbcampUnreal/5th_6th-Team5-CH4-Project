#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "T5PlayerState.h"
#include "T5PlayerController.generated.h"

UCLASS()
class TEAM5_PROJECT_API AT5PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// 역할 부여 신호 (서버 -> 클라)
	UFUNCTION(Client, Reliable)
	void Client_SetRole(EPlayerRole NewRole);

	// 단순 메시지 출력 (접속 알림, 카운트다운용)
	UFUNCTION(Client, Reliable)
	void Client_PrivateMessage(const FString& Message, FColor Color, int32 MsgKey = -1);

	// 치트 등 기존 함수 유지
	UFUNCTION(Exec) void CheatHit();
	UFUNCTION(Server, Reliable) void Server_CheatHit();
};