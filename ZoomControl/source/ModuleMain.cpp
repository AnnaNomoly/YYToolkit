#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;
static double zoom_level;
static bool zoom_obtained_from_title;

int GetMaxZoomOut()
{
	RValue window_width = g_ModuleInterface->CallBuiltin("window_get_width", {});
	RValue window_height = g_ModuleInterface->CallBuiltin("window_get_height", {});

	if (window_width.m_Real <= 1280 && window_height.m_Real <= 800)
	{
		return 2;
	}
	else if (window_width.m_Real <= 1920 && window_height.m_Real <= 1080)
	{
		return 3;
	}
	else if (window_width.m_Real <= 2560 && window_height.m_Real <= 1440)
	{
		return 5;
	}
	else /*if (window_width.m_Real <= 3840 && window_height.m_Real <= 2160)*/ {
		return 8;
	}
}

void ObjectCallback(
	IN FWCodeEvent& CodeEvent
)
{
	auto& [self, other, code, argc, argv] = CodeEvent.Arguments();

	if (!self)
		return;

	if (!self->m_Object)
		return;

	if (!strstr(self->m_Object->m_Name, "obj_ari"))
		return;
}

RValue& GmlScriptSetupMainScreenCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	zoom_obtained_from_title = false;

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_setup_main_screen@TitleMenu@TitleMenu"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptUpdateDisplay(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	int max_zoom_out = GetMaxZoomOut();
	bool modify_zoom = false;

	if (GetAsyncKeyState(VK_OEM_MINUS) & 1)
	{
		zoom_level += 1.0;
		if (zoom_level > max_zoom_out)
		{
			zoom_level = max_zoom_out;
		}
		modify_zoom = true;
	}

	if (GetAsyncKeyState(VK_OEM_PLUS) & 1)
	{
		zoom_level -= 1.0;
		if (zoom_level < 0) {
			zoom_level = 0;
		}
		modify_zoom = true;
	}

	if (modify_zoom) {
		CScript* gml_script_resize_amount = nullptr;
		g_ModuleInterface->GetNamedRoutinePointer(
			"gml_Script_set_expansion@Display@Display",
			(PVOID*)&gml_script_resize_amount
		);

		RValue out;
		RValue* in = new RValue(zoom_level);
		gml_script_resize_amount->m_Functions->m_ScriptFunction(
			Self,
			Other,
			out,
			1,
			{ &in }
		);

		delete in;
		modify_zoom = false;

		g_ModuleInterface->Print(CM_LIGHTGREEN, "[Zoom Control] - Set zoom-out level to: %f", zoom_level);
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_update@Display@Display"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);
	return Result;
}

RValue& GmlScriptSetExpansionCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if (!zoom_obtained_from_title) {
		zoom_level = Arguments[0]->m_Real;
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_set_expansion@Display@Display"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);
	return Result;
}

void CreateHookGmlScriptSetExpansion(AurieStatus& status)
{
	CScript* gml_script_get_move_speed = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_set_expansion@Display@Display",
		(PVOID*)&gml_script_get_move_speed
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Zoom Control] - Failed to get script (gml_Script_set_expansion@Display@Display)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_set_expansion@Display@Display",
		gml_script_get_move_speed->m_Functions->m_ScriptFunction,
		GmlScriptSetExpansionCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Zoom Control] - Failed to hook script (gml_Script_set_expansion@Display@Display)!");
	}
}

void CreateHookGmlScriptSetupMainScreen(AurieStatus& status)
{
	CScript* gml_script_setup_main_screen = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_setup_main_screen@TitleMenu@TitleMenu",
		(PVOID*)&gml_script_setup_main_screen
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Zoom Control] - Failed to get script (gml_Script_setup_main_screen@TitleMenu@TitleMenu)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_setup_main_screen@TitleMenu@TitleMenu",
		gml_script_setup_main_screen->m_Functions->m_ScriptFunction,
		GmlScriptSetupMainScreenCallback,
		nullptr
	);


	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Zoom Control] - Failed to hook script (gml_Script_setup_main_screen@TitleMenu@TitleMenu)!");
	}
}

void CreateHookGmlScriptUpdateDisplay(AurieStatus& status)
{
	CScript* gml_script_setup_main_screen = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_update@Display@Display",
		(PVOID*)&gml_script_setup_main_screen
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Zoom Control] - Failed to get script (gml_Script_update@Display@Display)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_update@Display@Display",
		gml_script_setup_main_screen->m_Functions->m_ScriptFunction,
		GmlScriptUpdateDisplay,
		nullptr
	);


	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Zoom Control] - Failed to hook script (gml_Script_update@Display@Display)!");
	}
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus status = AURIE_SUCCESS;

	status = ObGetInterface(
		"YYTK_Main",
		(AurieInterfaceBase*&)(g_ModuleInterface)
	);

	if (!AurieSuccess(status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[Zoom Control] - Hello from PluginEntry!");

	CreateHookGmlScriptSetExpansion(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Zoom Control] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptSetupMainScreen(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Zoom Control] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptUpdateDisplay(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Zoom Control] - Exiting due to failure on start!");
		return status;
	}

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[Zoom Control] - Failed to register callback!");
	}

	return AURIE_SUCCESS;
}