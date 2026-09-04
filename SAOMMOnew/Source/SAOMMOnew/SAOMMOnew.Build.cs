// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SAOMMOnew : ModuleRules
{
	public SAOMMOnew(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"HeadMountedDisplay",
			"AIModule",
			"GameplayTags",
			"NavigationSystem",
			"UMG",
			"Slate",
			"SlateCore"
		});

		// Note: EnvironmentQuery (EQS) was merged into AIModule in UE 5.8, so its
		// headers are reachable through the AIModule dependency above.
		PrivateDependencyModuleNames.AddRange(new string[] {
			"StateTreeModule",
			"GameplayStateTreeModule"
		});

		// UBT resolves PublicIncludePaths relative to the Source/ directory
		// (i.e. Source/SAOMMOnew/Core), so every entry needs the "SAOMMOnew/"
		// prefix. Listing every subfolder lets both pathless includes (e.g.
		// "SAOMMOCoreTypes.h") and prefixed ones (e.g.
		// "Variant_Combat/CombatGameMode.h") resolve.
		PublicIncludePaths.AddRange(new string[] {
			".",
			"SAOMMOnew",
			"SAOMMOnew/Core",
			"SAOMMOnew/Input",
			"SAOMMOnew/Character",
			"SAOMMOnew/Combat",
			"SAOMMOnew/Enemies",
			"SAOMMOnew/VR",
			"SAOMMOnew/Interaction",
			"SAOMMOnew/Inventory",
			"SAOMMOnew/Progression",
			"SAOMMOnew/World",
			"SAOMMOnew/Variant_Platforming",
			"SAOMMOnew/Variant_Platforming/Animation",
			"SAOMMOnew/Variant_Combat",
			"SAOMMOnew/Variant_Combat/AI",
			"SAOMMOnew/Variant_Combat/Animation",
			"SAOMMOnew/Variant_Combat/Gameplay",
			"SAOMMOnew/Variant_Combat/Interfaces",
			"SAOMMOnew/Variant_Combat/UI",
			"SAOMMOnew/Variant_SideScrolling",
			"SAOMMOnew/Variant_SideScrolling/AI",
			"SAOMMOnew/Variant_SideScrolling/Gameplay",
			"SAOMMOnew/Variant_SideScrolling/Interfaces",
			"SAOMMOnew/Variant_SideScrolling/UI"
		});
	}
}
