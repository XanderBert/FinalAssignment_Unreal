using UnrealBuildTool;

public class AbilitySystemEditor: ModuleRules
{
    public AbilitySystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "Core", 
            "CoreUObject", 
            "Engine",
            "AssetTools",
            "UnrealEd",
            "AbilitySystem",
        });
    }
}
