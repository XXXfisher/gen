using UnrealBuildTool;
using System.IO;

public class gen : ModuleRules
{
    public gen(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore",
            "EnhancedInput", "ProceduralMeshComponent", "Kismet"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        // KissFFT (compile from source .c files)
        string KissFFTPath = Path.Combine(ModuleDirectory, "ThirdParty", "KissFFT");

        PublicIncludePaths.Add(KissFFTPath);

        PrivateDependencyModuleNames.AddRange(new string[] { });

        
        PublicDefinitions.Add("KISS_FFT_STATIC=1");
    }
}
