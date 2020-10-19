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

#if WITH_EDITOR

#include "../Public/GrassPluginEdModeToolkit.h"
#include "../Public/GrassPluginEdMode.h"
#include "../Public/GrassRendering.h"

#include "Engine/Selection.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
//#if WITH_EDITOR
#include "EditorModeManager.h"
//#endif

#undef LOCTEXT_NAMESPACE
DEFINE_LOG_CATEGORY(LogGrassModeWidget);
#define LOCTEXT_NAMESPACE "FGrassPluginEdModeToolkit"

FGrassPluginEdModeToolkit::FGrassPluginEdModeToolkit()
{

}
FGrassPluginEdModeToolkit::~FGrassPluginEdModeToolkit()
{
	FPropertyEditorModule& propertyEditorModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	propertyEditorModule.UnregisterCustomClassLayout(FName("GrassPluginCustomLayout"));
}

void FGrassPluginEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{

	struct Locals
	{
		static bool IsWidgetEnabled()
		{
			return GEditor->GetSelectedActors()->Num() != 0;
		}

		static FReply OnButtonClickGrass()
		{
			UGrassRendering* render = ((FGrassPluginEdMode*)(GLevelEditorModeTools().GetActiveMode(FGrassPluginEdMode::EM_GrassPluginEdModeId)))->edModeSettings;
			render->SpawnPatchIfNotSpawned();
			if (render->overridePrevious)
				render->ClearGrass();
			
			if (render->poissonDisk)
				render->SpawnGrassBladesInTurfs();
			else
				render->SpawnGrassBladesCompletelyRandDistr();

			return FReply::Handled();
		}

		static FReply OnButtonClickClear() 
		{
			UGrassRendering* render = ((FGrassPluginEdMode*)(GLevelEditorModeTools().GetActiveMode(FGrassPluginEdMode::EM_GrassPluginEdModeId)))->edModeSettings;
			render->ClearGrass();
			return FReply::Handled();
		}

		static TSharedRef<SWidget> MakeButton(FText InLabel, int function)
		{
			switch (function) {
			case (0):
				return SNew(SButton)
					.Text(InLabel)
					.OnClicked_Static(&Locals::OnButtonClickGrass);
			default:
				return SNew(SButton)
					.Text(InLabel)
					.OnClicked_Static(&Locals::OnButtonClickClear);
			}
		}
	};
	//SETTINGS PANEL
	FPropertyEditorModule& propertyEditorModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs detailsViewArgs(false, false, false, FDetailsViewArgs::HideNameArea);

	propertyEditorModule.RegisterCustomClassLayout(
		FName("GrassRendering"),
		FOnGetDetailCustomizationInstance::CreateStatic(&FGrassPluginEdModeDetails::MakeInstance));

	// Create a new detailspanel
	auto detailsPanel = propertyEditorModule.CreateDetailView(detailsViewArgs);

	detailsPanel->OnFinishedChangingProperties().AddRaw(this, &FGrassPluginEdModeToolkit::ChangeVariables);

	FGrassPluginEdMode* grassMode = (FGrassPluginEdMode*)GetEditorMode(); // addEditorMode
	if (grassMode)
	{
		detailsPanel->SetObject(grassMode->edModeSettings, true);
	}

	SAssignNew(ToolkitWidget, SBorder)
		.HAlign(HAlign_Center)
		.Padding(25)
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(50)
					[
						detailsPanel
					]
				+ SVerticalBox::Slot()
					.HAlign(HAlign_Center)
					.AutoHeight()
					[
						Locals::MakeButton(LOCTEXT("Spawn Grass", "Spawn Grass"), 0)
					]
				+ SVerticalBox::Slot()
					.HAlign(HAlign_Center)
					.AutoHeight()
					[
						Locals::MakeButton(LOCTEXT("Clear Grass", "Clear"), 1)
					]
			]
		];

	propertyEditorModule.NotifyCustomizationModuleChanged();
	FModeToolkit::Init(InitToolkitHost);
}

FName FGrassPluginEdModeToolkit::GetToolkitFName() const
{
	return FName("GrassPluginEdMode");
}

FText FGrassPluginEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("GrassPluginEdModeToolkit", "DisplayName", "GrassPluginEdMode Tool");
}

class FEdMode* FGrassPluginEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FGrassPluginEdMode::EM_GrassPluginEdModeId);
}

void FGrassPluginEdModeToolkit::ChangeVariables(const FPropertyChangedEvent & callback)
{
	FGrassPluginEdMode* grassMode = (FGrassPluginEdMode*)GLevelEditorModeTools().GetActiveMode(FGrassPluginEdMode::EM_GrassPluginEdModeId);
	if (grassMode) {
		if (callback.GetPropertyName().ToString().Equals("grassShape"))
			grassMode->edModeSettings->RefreshGrassMode();
	}
	

}

#undef LOCTEXT_NAMESPACE
#endif