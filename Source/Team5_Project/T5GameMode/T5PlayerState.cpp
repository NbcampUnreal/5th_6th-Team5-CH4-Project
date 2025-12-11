// T5PlayerState.cpp

#include "T5PlayerState.h"
#include "Net/UnrealNetwork.h"

AT5PlayerState::AT5PlayerState()
{
    // 변수 초기화
    CurrentRole = EPlayerRole::None;
    bReplicates = true;
}

void AT5PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // CurrentRole 변수를 동기화 목록에 추가
    DOREPLIFETIME(AT5PlayerState, CurrentRole);
}
