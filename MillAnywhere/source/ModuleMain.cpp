#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;

RValue& GmlScriptHeldItemCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if (GetAsyncKeyState(VK_F12) & 1)
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[MillAnywhere] - Opening Mill menu!");

		CScript* gml_script_mill_menu = nullptr;
		g_ModuleInterface->GetNamedRoutinePointer(
			"gml_Script_anon@382@gml_Object_obj_mill_menu_Create_0",
			(PVOID*)&gml_script_mill_menu
		);

		RValue retval;
		gml_script_mill_menu->m_Functions->m_ScriptFunction(
			Self,
			Other,
			retval,
			0,
			nullptr
		);
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_held_item@Ari@Ari"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

void CreateHookGmlScriptHeldItem(AurieStatus& status)
{
	CScript* gml_script_held_item = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_held_item@Ari@Ari",
		(PVOID*)&gml_script_held_item
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[MillAnywhere] - Failed to get script (gml_Script_held_item@Ari@Ari)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_held_item@Ari@Ari",
		gml_script_held_item->m_Functions->m_ScriptFunction,
		GmlScriptHeldItemCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[MillAnywhere] - Failed to hook script (gml_Script_held_item@Ari@Ari)!");
	}
}

EXPORTED AurieStatus ModuleInitialize(IN AurieModule* Module, IN const fs::path& ModulePath) {
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus status = AURIE_SUCCESS;
	
	status = ObGetInterface(
		"YYTK_Main", 
		(AurieInterfaceBase*&)(g_ModuleInterface)
	);

	if (!AurieSuccess(status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTYELLOW, "[MillAnywhere] - Plugin loading...");
	
	CreateHookGmlScriptHeldItem(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[MillAnywhere] - Exiting due to failure on start!");
		return status;
	}

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[MillAnywhere] - Plugin loaded!");
	return AURIE_SUCCESS;
}