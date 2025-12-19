#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "T5PlayerState.h"
//#include "T5GameMode/T5GameState.h"
#include "T5PlayerController.generated.h"

UCLASS()
class TEAM5_PROJECT_API AT5PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Client, Reliable)
	void Client_SetRole(EPlayerRole NewRole);

	UFUNCTION(Client, Reliable)
	void Client_PrivateMessage(const	 FString& Msg, FColor Color, int32 Key = -1);

	/*UFUNCTION(Client, Reliable)
	void Client_OnGameOver(EWinningTeam WinningTeam);*/
	
};