using UnrealBuildTool;

public class ZeonEditor : ModuleRules
{
	public ZeonEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			// UI/Slate
			"Slate", "SlateCore",
			// Меню и кнопки тулбаров
			"ToolMenus",
			// Делегаты редактора, PIE и пр.
			"UnrealEd",
			// Если расширяете главное меню/панели редактора
			"LevelEditor",
			// Нередко требуется для команд/действий
			"EditorFramework",
			// Часто нужен при регистрации/иконках/плагин-инфо
			"Projects"
		});
	}
}