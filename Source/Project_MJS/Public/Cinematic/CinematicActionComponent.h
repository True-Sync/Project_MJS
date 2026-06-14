#pragma once

#include "CoreMinimal.h"
#include "Cinematic/CinematicTypes.h"
#include "Components/ActorComponent.h"
#include "CinematicActionComponent.generated.h"

class ULevelSequence;

/*
 * UCinematicActionComponent
 *	액터가 코드나 블루프린트에서 시네마틱을 쉽게 요청하게 해주는 래퍼 컴포넌트. 
 *	플레이어 스킬/궁극기 쪽에서 PlayCinematicRequest나 PlayAnchoredCinematic으로 Director에 요청을 보낸다.
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_MJS_API UCinematicActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCinematicActionComponent();

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	bool PlayCinematic(
		ULevelSequence* Sequence,
		bool bAffectAllParticipants = true,
		bool bRestoreViewTarget = true,
		float BlendOutTime = 0.15f);

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	bool PlayCinematicRequest(FCinematicPlaybackRequest Request);

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	bool PlayAnchoredCinematic(
		ULevelSequence* Sequence,
		ECinematicAnchorMode AnchorMode,
		ECinematicRotationSource RotationSource,
		FTransform RelativeTransform,
		bool bUseYawOnly = true,
		bool bAffectAllParticipants = true,
		bool bRestoreViewTarget = true,
		float BlendOutTime = 0.15f);

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void StopCinematic();

	UFUNCTION(BlueprintPure, Category = "Cinematic")
	bool IsCinematicPlaying() const;

private:
	bool SubmitCinematicRequest(FCinematicPlaybackRequest& Request) const;
};
