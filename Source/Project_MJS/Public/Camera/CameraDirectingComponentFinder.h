#pragma once

#include "CoreMinimal.h"

class UCameraDirectingComponent;
class USkeletalMeshComponent;

PROJECT_MJS_API UCameraDirectingComponent* FindCameraDirectingComponentFromMesh(const USkeletalMeshComponent* MeshComp);
