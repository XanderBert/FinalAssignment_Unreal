#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions/AssetTypeActions_Blueprint.h"
#include "AssetDefinition_AbilityDescription.generated.h"

class ABILITYSYSTEMEDITOR_API FAssetTypeActions_AbilityDescription : public FAssetTypeActions_Blueprint
{
public:
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Blueprint | EAssetTypeCategories::Gameplay; }

	//Used to do certain things or initialize things when we open up the editor for the object
	//virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;


	virtual UFactory* GetFactoryForBlueprintType(UBlueprint* InBlueprint) const override;

private:

	/** Returns true if the blueprint is data only */
	bool ShouldUseDataOnlyEditor(const UBlueprint* Blueprint) const;
};

UCLASS(hidecategories = Object, CollapseCategories,  MinimalAPI)
class UAbilityDescriptionFactory : public UFactory
{
	GENERATED_UCLASS_BODY()
public:
	// The type of blueprint that will be created
	UPROPERTY(EditAnywhere, Category = GameplayAbilitiesBlueprintFactory)
	TEnumAsByte<enum EBlueprintType> BlueprintType = BPTYPE_Normal;

	// The parent class of the created blueprint
	UPROPERTY(EditAnywhere, Category = GameplayAbilitiesBlueprintFactory)
	TSubclassOf<class UAbilityDescription> ParentClass;


	//virtual bool ConfigureProperties() override; // Mainly used to spawn a dialog when we want to. For example select our base class for the blueprint but you can configure anything you want.
	
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};