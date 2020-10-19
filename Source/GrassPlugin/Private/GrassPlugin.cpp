// Copyright 2020 Matous Prochazka, Bohemia Interactive, a.s.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright notice,
//      this list of conditions and the following disclaimer in the documentation
//      and/or other materials provided with the distribution.
//
//   3. Neither the name of the copyright holder nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "../Public/GrassPlugin.h"
#if WITH_EDITOR
#include "../Public/GrassPluginEdMode.h"

#define LOCTEXT_NAMESPACE "FGrassPluginModule"

#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FGrassPluginModule::InContent( RelativePath, ".png" ), __VA_ARGS__ )

FString FGrassPluginModule::InContent(const FString & RelativePath, const ANSICHAR * Extension)
{
	static FString contentDir = FPaths::ProjectPluginsDir() + TEXT("GrassPlugin/Content");
	return (contentDir / RelativePath) + Extension;
}

TSharedPtr< FSlateStyleSet > FGrassPluginModule::StyleSet = nullptr;
TSharedPtr< class ISlateStyle > FGrassPluginModule::Get() { return StyleSet; }

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FGrassPluginModule::Initialize()
{
	// Const icon sizes
	const FVector2D icon8x8(8.0f, 8.0f);
	const FVector2D icon16x16(16.0f, 16.0f);
	const FVector2D icon20x20(20.0f, 20.0f);
	const FVector2D icon40x40(40.0f, 40.0f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));
	StyleSet->Set("LevelEditor.GrassPlugin", new IMAGE_PLUGIN_BRUSH("GrassTurf2", icon40x40));
	StyleSet->Set("LevelEditor.GrassPlugin.Small", new IMAGE_PLUGIN_BRUSH("GrassTurf2", icon20x20));
	StyleSet->Set("LevelEditor.GrassPlugin.Selected", new IMAGE_PLUGIN_BRUSH("GrassTurf2", icon40x40));
	StyleSet->Set("LevelEditor.GrassPlugin.Selected.Small", new IMAGE_PLUGIN_BRUSH("GrassTurf2", icon20x20));


	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef IMAGE_PLUGIN_BRUSH


FName FGrassPluginModule::GetStyleSetName()
{
	static FName styleName(TEXT("GrassPlugin"));
	return styleName;
}

void FGrassPluginModule::StartupModule()
{
	Initialize();
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FGrassPluginEdMode>(FGrassPluginEdMode::EM_GrassPluginEdModeId, LOCTEXT("GrassPluginEdModeName", "GrassPluginEdMode"), FSlateIcon(GetStyleSetName(), "LevelEditor.GrassPlugin", "LevelEditor.GrassPlugin.Small"), true);
}

void FGrassPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FGrassPluginEdMode::EM_GrassPluginEdModeId);
}

#undef LOCTEXT_NAMESPACE


#endif
IMPLEMENT_MODULE(FGrassPluginModule, GrassPlugin)