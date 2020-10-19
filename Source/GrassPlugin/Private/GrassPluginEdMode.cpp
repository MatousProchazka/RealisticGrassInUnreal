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
#include "../Public/GrassPluginEdMode.h"
#include "../Public/GrassPluginEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

const FEditorModeID FGrassPluginEdMode::EM_GrassPluginEdModeId = TEXT("EM_GrassPluginEdMode");

FGrassPluginEdMode::FGrassPluginEdMode()
{
	edModeSettings = NewObject<UGrassRendering>(GetTransientPackage(), TEXT("GrassRendering"), RF_Transactional);
	edModeSettings->AddToRoot();
	edModeSettings->editorMode = EM_GrassPluginEdModeId;
	edModeSettings->OnNewMapLoad.BindRaw(this, &FGrassPluginEdMode::ReloadEditorMode);
}

FGrassPluginEdMode::~FGrassPluginEdMode()
{
	edModeSettings->RemoveFromRoot();
}

void FGrassPluginEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FGrassPluginEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FGrassPluginEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}
	
	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FGrassPluginEdMode::UsesToolkits() const
{
	return true;
}

void FGrassPluginEdMode::ReloadEditorMode()
{
	FEditorModeTools tools = GLevelEditorModeTools();
	tools.ActivateMode(EM_GrassPluginEdModeId, true);
	Enter();
	tools.BroadcastEditorModeChanged(tools.FindMode(EM_GrassPluginEdModeId), true);
}



#endif
