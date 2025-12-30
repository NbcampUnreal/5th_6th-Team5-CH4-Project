// Source/Team5_Project/UI/T5UIGameModeBase.cpp

#include "UI/T5UIGameModeBase.h"
#include "T5GameMode/T5GameInstance.h" // 인스턴스 헤더 필수!

AT5UIGameModeBase::AT5UIGameModeBase()
{
	// 클라이언트가 서버를 따라오게 만드는 핵심 설정
	bUseSeamlessTravel = true;
}

// [여기가 핵심!] 버튼을 누르면 실행될 진짜 로직입니다.
void AT5UIGameModeBase::StartGameSequence()
{
	// 1. 현재 로비에 몇 명 있는지 세기
	int32 CurrentNumPlayers = GetNumPlayers();

	// 2. GameInstance(여행 가방)에 인원수 저장하기
	if (UT5GameInstance* GI = Cast<UT5GameInstance>(GetGameInstance()))
	{
		GI->TargetPlayerCount = CurrentNumPlayers;
		UE_LOG(LogTemp, Warning, TEXT(">>> [로비] 총 인원 %d명 확정! <<<"), CurrentNumPlayers);
	}

	// 3. 다 같이 맵 이동 (ServerTravel)
	// 경로에 오타 없는지 꼭 확인하세요!
	GetWorld()->ServerTravel(TEXT("/Game/KTE_MAP/L_TestGround?listen"));
}