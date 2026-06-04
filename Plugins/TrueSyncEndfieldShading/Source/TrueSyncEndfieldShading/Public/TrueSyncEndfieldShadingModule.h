#pragma once

#include "Modules/ModuleManager.h"

class FTrueSyncEndfieldShadingSceneViewExtension;

DECLARE_LOG_CATEGORY_EXTERN(LogTrueSyncEndfieldShading, Log, All);

class FTrueSyncEndfieldShadingModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void HandlePostEngineInit();
	void CreateSceneViewExtension();
	void ReleaseSceneViewExtension();

	FDelegateHandle PostEngineInitHandle;
	TSharedPtr<FTrueSyncEndfieldShadingSceneViewExtension, ESPMode::ThreadSafe> SceneViewExtension;
};
