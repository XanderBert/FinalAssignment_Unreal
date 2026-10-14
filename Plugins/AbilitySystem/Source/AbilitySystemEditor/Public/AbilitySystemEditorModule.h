#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FAbilitySystemEditor : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<class FAssetTypeActions_AbilityDescription> AbilityDescriptionAsset;
};
