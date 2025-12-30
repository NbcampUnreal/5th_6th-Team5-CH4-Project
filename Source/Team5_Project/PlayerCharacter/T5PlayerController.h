#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "T5PlayerState.h"
#include "T5GameMode/T5GameState.h"
#include "T5PlayerController.generated.h"

class UUserWidget;

UCLASS()
class TEAM5_PROJECT_API AT5PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// 역할 지정
	UFUNCTION(Client, Reliable)
	void Client_SetRole(EPlayerRole NewRole);

	// 개인 메시지
	UFUNCTION(Client, Reliable)
	void Client_PrivateMessage(const FString& Msg, FColor Color, int32 Key = -1);

	// 게임 종료 (승패 판정)
	UFUNCTION(Client, Reliable)
	void Client_OnGameOver(EWinningTeam WinningTeam);

	// 죽었을 때 Die UI
	UFUNCTION(Client, Reliable)
	void Client_ShowDieUI();

	// BP 이벤트
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnRoleAssigned(EPlayerRole NewRole);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnGameOver(EWinningTeam WinningTeam);

	// 메인 메뉴로 나가기
	UFUNCTION(BlueprintCallable, Category="UI")
	void Request_ExitToMainMenu();

protected:
	// Die UI 클래스 (BP에서 지정)
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> DieWidgetClass;

private:
	// Die UI 인스턴스
	UPROPERTY()
	UUserWidget* DieWidgetInstance = nullptr;

	//  이 로컬 플레이어가 이미 죽었는지 여부
	UPROPERTY()
	bool bLocalIsDead = false;
};
