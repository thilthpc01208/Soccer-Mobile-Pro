#include "ProjectSoccerEditor.h"
#include "UnrealEd.h"

IMPLEMENT_MODULE(FProjectSoccerEditorModule, ProjectSoccerEditor);

void FProjectSoccerEditorModule::StartupModule()
{
    // Register the Debug View Data Table Asset's Actions:
    DebugViewDataTableAssetTypeActions = MakeShared<FDebugViewDataTableAssetTypeActions>();
    FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(DebugViewDataTableAssetTypeActions.ToSharedRef());
}

void FProjectSoccerEditorModule::ShutdownModule()
{
    // Unregister the Debug View Data Table Asset's Actions:
    if (!FModuleManager::Get().IsModuleLoaded("AssetTools"))
        return;

    FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(DebugViewDataTableAssetTypeActions.ToSharedRef());
}

