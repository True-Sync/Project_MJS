#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Cinematic/CinematicTypes.h"
#include "CinematicParticipant.generated.h"

/*
* ICinematicParticipant
*	시네마틱 시작/종료 알림을 받을 수 있는 인터페이스. 
*	액터나 컴포넌트가 이걸 꼭 구현해야만 Director가 OnCinematicStarted, OnCinematicEnded를 호출한다.
*/
UINTERFACE(BlueprintType)
class PROJECT_MJS_API UCinematicParticipant : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_MJS_API ICinematicParticipant
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Cinematic")
	void OnCinematicStarted(const FCinematicPlaybackContext& Context);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Cinematic")
	void OnCinematicEnded(const FCinematicPlaybackContext& Context);
};
