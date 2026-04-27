#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "ProjectSoccerEditor/Public/Tools/DebugViewDataTableAssetTypeActions.h"

class FProjectSoccerEditorModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    TSharedPtr<FDebugViewDataTableAssetTypeActions> DebugViewDataTableAssetTypeActions;
};