#include "T5PlayerController.h"
#include "T5GameMode/T5GameMode.h"
#include "T5PlayerState.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void AT5PlayerController::Client_SetRole_Implementation(EPlayerRole NewRole)
{
	if (!IsLocalController()) return;

	AT5PlayerState* PS = GetPlayerState<AT5PlayerState>();
	if (PS == nullptr || PS->GetPlayerId() == -1)
	{
		FTimerHandle WaitTimer;
		GetWorld()->GetTimerManager().SetTimer(
			WaitTimer,
			[this, NewRole]() { Client_SetRole(NewRole); },
			0.1f, false
		);
		return;
	}

	BP_OnRoleAssigned(NewRole);
}

void AT5PlayerController::Client_PrivateMessage_Implementation(
	const FString& Msg, FColor Color, int32 Key)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(Key, 2.0f, Color, Msg);
	}
}

void AT5PlayerController::Client_OnGameOver_Implementation(EWinningTeam Winner)
{
	if (!IsLocalController()) return;
	
	// 이미 죽어서 Die UI가 떠 있는 플레이어라면
	// GameOver UI(Win/Lose)를 절대 띄우지 않는다
	if (bLocalIsDead)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Client_OnGameOver] Local player is dead -> skip GameOver UI"));
		return;
	}

	// 캐릭터 조작 완전 정지
	if (APawn* MyPawn = GetPawn())
	{
		MyPawn->DisableInput(this);

		if (ACharacter* MyChar = Cast<ACharacter>(MyPawn))
		{
			if (MyChar->GetCharacterMovement())
			{
				MyChar->GetCharacterMovement()->StopMovementImmediately();
				MyChar->GetCharacterMovement()->DisableMovement();
			}
		}
	}

	// UI 입력 모드
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());

	FString WinStr =
		(Winner == EWinningTeam::Hunter) ? TEXT("술래 승리!") : TEXT("도망자 승리!");
	UE_LOG(LogTemp, Warning, TEXT(">>> [게임 종료] %s <<<"), *WinStr);

	// BP에서 승/패 UI 띄우기
	BP_OnGameOver(Winner);
}

void AT5PlayerController::Client_ShowDieUI_Implementation()
{
	if (!IsLocalController()) return;

	// 이 플레이어는 죽었다고 확정
	bLocalIsDead = true;

	// 중복 생성 방지
	if (DieWidgetInstance) return;

	if (!DieWidgetClass)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Client_ShowDieUI] DieWidgetClass is null. Set it in BP_T5PlayerController."));
		return;
	}

	DieWidgetInstance = CreateWidget<UUserWidget>(this, DieWidgetClass);
	if (!DieWidgetInstance) return;

	DieWidgetInstance->AddToViewport(200);

	bShowMouseCursor = true;

	FInputModeUIOnly Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
}

void AT5PlayerController::Request_ExitToMainMenu()
{
	if (!IsLocalController()) return;

	const FString MainMenuMap = TEXT("/Game/LJH_UI/MainmenuLevel");
	ClientTravel(MainMenuMap, ETravelType::TRAVEL_Absolute);
}
