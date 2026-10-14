#include "AssetDefinition_AbilityDescription.h"
#include "EnhancedAbilitySystemComponent.h"
#include <Kismet2/KismetEditorUtilities.h>
#include "Kismet2/BlueprintEditorUtils.h"
#include <KismetCompilerModule.h>


#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_AbilityDescription::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AbilityDescriptionBlueprint", "Ability Description Blueprint");
}

FColor FAssetTypeActions_AbilityDescription::GetTypeColor() const
{
	return FColor::FromHex("#C9A0DCFF");
}

UClass* FAssetTypeActions_AbilityDescription::GetSupportedClass() const
{
	return UAbilityDescriptionBlueprint::StaticClass();
}

UFactory* FAssetTypeActions_AbilityDescription::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UAbilityDescriptionFactory* AbilityDescriptionFactory = NewObject<UAbilityDescriptionFactory>();
	AbilityDescriptionFactory->ParentClass = TSubclassOf<UAbilityDescription>(*InBlueprint->GeneratedClass);
	return AbilityDescriptionFactory;
}

bool FAssetTypeActions_AbilityDescription::ShouldUseDataOnlyEditor(const UBlueprint* Blueprint) const
{
	return FBlueprintEditorUtils::IsDataOnlyBlueprint(Blueprint)
		&& !FBlueprintEditorUtils::IsLevelScriptBlueprint(Blueprint)
		&& !FBlueprintEditorUtils::IsInterfaceBlueprint(Blueprint)
		&& !Blueprint->bForceFullEditor
		&& !Blueprint->bIsNewlyCreated;
}


UAbilityDescriptionFactory::UAbilityDescriptionFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UAbilityDescriptionBlueprint::StaticClass();
	ParentClass = UAbilityDescription::StaticClass();
}


UObject* UAbilityDescriptionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	// Make sure we are trying to factory a gameplay ability blueprint, then create and init one
	check(InClass->IsChildOf(UAbilityDescriptionBlueprint::StaticClass()));


	// If they selected an interface, force the parent class to be UInterface
	if (BlueprintType == BPTYPE_Interface)
	{
		ParentClass = UInterface::StaticClass();
	}

	if ((ParentClass == NULL) || !FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass) || !ParentClass->IsChildOf(UAbilityDescription::StaticClass()))
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ClassName"), (ParentClass != NULL) ? FText::FromString(ParentClass->GetName()) : LOCTEXT("Null", "(null)"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("CannotCreateAbilityDescriptionBlueprint", "Cannot create a Gameplay Ability Description Blueprint based on the class '{ClassName}'."), Args));
		return NULL;
	}
	else
	{
		UClass* BlueprintClass = nullptr;
		UClass* BlueprintGeneratedClass = nullptr;

		IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
		KismetCompilerModule.GetBlueprintTypesForClass(ParentClass, BlueprintClass, BlueprintGeneratedClass);

		return FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, InName, BlueprintType, UAbilityDescriptionBlueprint::StaticClass(), BlueprintGeneratedClass, CallingContext);

	}

	return NULL;
}

UObject* UAbilityDescriptionFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, NAME_None);
}

#undef LOCTEXT_NAMESPACE