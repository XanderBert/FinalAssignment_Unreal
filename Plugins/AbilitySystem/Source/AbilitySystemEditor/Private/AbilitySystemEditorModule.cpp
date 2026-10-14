#include "AbilitySystemEditorModule.h"
#include "Modules/ModuleManager.h"
#include <AssetToolsModule.h>
#include <AssetDefinition_AbilityDescription.h>

IMPLEMENT_MODULE(FAbilitySystemEditor, AbilitySystemEditor);

void FAbilitySystemEditor::StartupModule()
{
	AbilityDescriptionAsset = MakeShareable(new FAssetTypeActions_AbilityDescription);

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.RegisterAssetTypeActions(AbilityDescriptionAsset.ToSharedRef());
}

void FAbilitySystemEditor::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(AbilityDescriptionAsset.ToSharedRef());
	}

	if (AbilityDescriptionAsset.IsValid()) 
	{
		// Release the shared pointer
		AbilityDescriptionAsset.Reset();
	}
}