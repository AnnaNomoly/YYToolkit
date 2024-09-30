#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;
static int ari_actual_stamina = -1;
static int health_deficit = 0;

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


	if (health_deficit != 0)
	{
		CInstance* global_instance = nullptr;
		g_ModuleInterface->GetGlobalInstance(&global_instance);

		CScript* gml_script_modify_health = nullptr;
		g_ModuleInterface->GetNamedRoutinePointer(
			"gml_Script_modify_health@Ari@Ari",
			(PVOID*)&gml_script_modify_health
		);

		RValue result;
		RValue* health_penalty = new RValue(health_deficit);

		gml_script_modify_health->m_Functions->m_ScriptFunction(
			global_instance->at("__ari").m_Object,
			self,
			result,
			1,
			{ &health_penalty }
		);

		health_deficit = 0;
		delete health_penalty;
	}
}

RValue& GmlScriptGetStaminaCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_get_stamina@Ari@Ari"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	ari_actual_stamina = Result.m_Real;
	if (ari_actual_stamina == 0)
	{
		Result.m_Real = 1;
	}

	return Result;
}

RValue& GmlScriptModifyStaminaCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	int stamina_offset = 0;
	if (ari_actual_stamina == 0)
	{
		stamina_offset -= 1;
	}

	int temp = stamina_offset + ari_actual_stamina + Arguments[0]->m_Real;
	if (temp < 0)
	{
		ari_actual_stamina = 0;
		health_deficit += Arguments[0]->m_Real;
	}
	else {
		ari_actual_stamina += Arguments[0]->m_Real;
	}
	
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_modify_stamina@Ari@Ari"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

void CreateHookGmlScriptGetStamina(AurieStatus& status)
{
	CScript* gml_script = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_get_stamina@Ari@Ari",
		(PVOID*)&gml_script
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Failed to get script (gml_Script_get_stamina@Ari@Ari)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_get_stamina@Ari@Ari",
		gml_script->m_Functions->m_ScriptFunction,
		GmlScriptGetStaminaCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Failed to hook script (gml_Script_get_stamina@Ari@Ari)!");
	}
}

void CreateHookGmlScriptModifyStamina(AurieStatus& status)
{
	CScript* gml_script_modify_stamina = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_modify_stamina@Ari@Ari",
		(PVOID*)&gml_script_modify_stamina
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Failed to get script (gml_Script_modify_stamina@Ari@Ari)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_modify_stamina@Ari@Ari",
		gml_script_modify_stamina->m_Functions->m_ScriptFunction,
		GmlScriptModifyStaminaCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Failed to hook script (gml_Script_modify_stamina@Ari@Ari)!");
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

	g_ModuleInterface->Print(CM_LIGHTYELLOW, "[WorkHard] - Plugin starting...");

	g_ModuleInterface->CreateCallback(g_ArSelfModule, EVENT_OBJECT_CALL, ObjectCallback, 0);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[WorkHard] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptGetStamina(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[WorkHard] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptModifyStamina(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[WorkHard] - Exiting due to failure on start!");
		return status;
	}

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[WorkHard] - Failed to register callback!");
	}

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[WorkHard] - Plugin started!");

	return AURIE_SUCCESS;
}