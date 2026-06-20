#include "TrueSyncEndfieldShadingModule.h"

#include "TrueSyncEndfieldShadingSceneViewExtension.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "RenderingThread.h"
#include "SceneViewExtension.h"
#include "ShaderCore.h"

DEFINE_LOG_CATEGORY(LogTrueSyncEndfieldShading);

IMPLEMENT_MODULE(FTrueSyncEndfieldShadingModule, TrueSyncEndfieldShading)

void FTrueSyncEndfieldShadingModule::StartupModule()
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("TrueSyncEndfieldShading"));
	if (ensure(Plugin.IsValid()))
	{
		const FString ShaderDirectory = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/Plugin/TrueSyncEndfieldShading"), ShaderDirectory);
	}

	if (GEngine != nullptr)
	{
		CreateSceneViewExtension();
	}
	else
	{
		PostEngineInitHandle = FCoreDelegates::GetOnPostEngineInit().AddRaw(
			this, &FTrueSyncEndfieldShadingModule::HandlePostEngineInit);
	}
}

void FTrueSyncEndfieldShadingModule::ShutdownModule()
{
	if (PostEngineInitHandle.IsValid())
	{
		FCoreDelegates::GetOnPostEngineInit().Remove(PostEngineInitHandle);
		PostEngineInitHandle.Reset();
	}

	ReleaseSceneViewExtension();
}

void FTrueSyncEndfieldShadingModule::HandlePostEngineInit()
{
	CreateSceneViewExtension();
}

void FTrueSyncEndfieldShadingModule::CreateSceneViewExtension()
{
	if (!SceneViewExtension.IsValid())
	{
		SceneViewExtension = FSceneViewExtensions::NewExtension<FTrueSyncEndfieldShadingSceneViewExtension>();
	}
}

void FTrueSyncEndfieldShadingModule::ReleaseSceneViewExtension()
{
	if (!SceneViewExtension.IsValid())
	{
		return;
	}

	SceneViewExtension->Invalidate();
	FlushRenderingCommands();
	SceneViewExtension.Reset();
}
