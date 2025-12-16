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

	// [테스트 코드] 치트 기능 (임시 비활성화)
	/*
	UFUNCTION(Exec) void CheatHit();
	UFUNCTION(Server, Reliable) void Server_CheatHit();
	*/
};