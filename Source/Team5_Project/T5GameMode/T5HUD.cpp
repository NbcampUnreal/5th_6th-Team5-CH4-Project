#include "T5HUD.h"
#include "Engine/Canvas.h"
#include "T5PlayerController.h"
#include "T5PlayerState.h"

void AT5HUD::DrawHUD()
{
	Super::DrawHUD();

	// 내 컨트롤러와 내 상태 가져오기
	AT5PlayerController* PC = Cast<AT5PlayerController>(GetOwningPlayerController());
	if (!PC) return;

	AT5PlayerState* PS = PC->GetPlayerState<AT5PlayerState>();
	if (!PS) return;

	// 출력할 텍스트 만들기
	FString RoleText;
	FColor TextColor = FColor::White;

	int32 MyId = PS->GetPlayerId();
	float MyHP = PS->CurrentHP;

	// 로딩 중일 때(-1)는 표시 안 하거나 로딩 중이라고 표시
	if (MyId == -1) 
	{
		DrawText(TEXT("Loading..."), FColor::Yellow, 50, 50, nullptr, 2.0f);
		return;
	}

	if (PS->CurrentRole == EPlayerRole::Hunter)
	{
		RoleText = FString::Printf(TEXT("[ID:%d] 역할: 술래 (HUNTER)\nHP: %.0f"), MyId, MyHP);
		TextColor = FColor::Red;
	}
	else if (PS->CurrentRole == EPlayerRole::Animal)
	{
		RoleText = FString::Printf(TEXT("[ID:%d] 역할: 도망자 (ANIMAL)\nHP: %.0f"), MyId, MyHP);
		TextColor = FColor::Green;
	}
	else
	{
		RoleText = TEXT("대기 중...");
		TextColor = FColor::Green;
	}

	// 화면 좌측 상단(50, 50) 좌표에 텍스트 그리기
	// 이 함수는 오직 '이 HUD를 소유한 플레이어 화면'에만
	DrawText(RoleText, TextColor, 50, 50, nullptr, 2.0f); // 2.0f는 글자 크기 배율
}