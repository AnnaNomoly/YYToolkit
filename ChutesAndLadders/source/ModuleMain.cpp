#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;

RValue& GmlScriptTryLocationIdToStringCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_try_location_id_to_string"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	if (GetAsyncKeyState(VK_NEXT) & 1)
	{
		std::string location = Result.AsString().data();
		if (location == "dungeon")
		{
			for (int i = 0; i < 40; i++)
			{
				for (int j = 0; j < 40; j++)
				{
					try
					{
						CScript* gml_Script_spawn_ladder = nullptr;
						g_ModuleInterface->GetNamedRoutinePointer(
							"gml_Script_spawn_ladder@DungeonRunner@DungeonRunner",
							(PVOID*)&gml_Script_spawn_ladder
						);

						RValue x = static_cast<double>(i);
						RValue y = static_cast<double>(j);
						RValue* x_ptr = &x;
						RValue* y_ptr = &y;
						RValue* rvalue_array[2] = { x_ptr, y_ptr };
						RValue retval;
						gml_Script_spawn_ladder->m_Functions->m_ScriptFunction(
							Self,
							Other,
							retval,
							2,
							rvalue_array
						);

						g_ModuleInterface->Print(CM_LIGHTGREEN, "[ChutesAndLadders] - Spawned ladder at (%d, %d)!", i, j);
						return Result;
					}
					catch (...) {}
				}
			}
			g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Found no suitable position for a ladder!");
		}
	}
	
	return Result;
}

RValue& GmlScriptErrorCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	std::string error = Arguments[0]->AsString().data();
	if (error == "Failed to spawn a ladder in the dungeon!")
	{
		return Result;
	}
	else
	{
		const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_error"));
		original(
			Self,
			Other,
			Result,
			ArgumentCount,
			Arguments
		);

		return Result;
	}
}

void CreateHookGmlScriptTryLocationIdToString(AurieStatus& status)
{
	CScript* gml_script_try_location_id_to_string = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_try_location_id_to_string",
		(PVOID*)&gml_script_try_location_id_to_string
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Failed to get script (gml_Script_try_location_id_to_string)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_try_location_id_to_string",
		gml_script_try_location_id_to_string->m_Functions->m_ScriptFunction,
		GmlScriptTryLocationIdToStringCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Failed to hook script (gml_Script_try_location_id_to_string)!");
	}
}

void CreateHookGmlScriptError(AurieStatus& status)
{
	CScript* gml_script_load_game = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_error",
		(PVOID*)&gml_script_load_game
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Failed to get script (gml_Script_error)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_error",
		gml_script_load_game->m_Functions->m_ScriptFunction,
		GmlScriptErrorCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Failed to hook script (gml_Script_error)!");
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

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[ChutesAndLadders] - Plugin starting...");
	
	CreateHookGmlScriptTryLocationIdToString(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptError(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Exiting due to failure on start!");
		return status;
	}

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[ChutesAndLadders] - Plugin started!");
	return AURIE_SUCCESS;
}