#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;
//static bool room_loaded = false;

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
			for (int i = 0; i < 100; i++)
			{
				for (int j = 0; i < 100; j++)
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
						i = INT_MAX - 1;
						j = INT_MAX - 1;
					}
					catch (...) {}
				}
			}


		}
	}
	
	return Result;
}

//RValue& GmlScriptShowRoomTitleCallback(
//	IN CInstance* Self,
//	IN CInstance* Other,
//	OUT RValue& Result,
//	IN int ArgumentCount,
//	IN RValue** Arguments
//)
//{
//	room_loaded = true;
//
//	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_show_room_title"));
//	original(
//		Self,
//		Other,
//		Result,
//		ArgumentCount,
//		Arguments
//	);
//
//	return Result;
//}

//void CreateHookGmlScriptSpawnLadder(AurieStatus& status)
//{
//	CScript* gml_Script_spawn_ladder = nullptr;
//	status = g_ModuleInterface->GetNamedRoutinePointer(
//		"gml_Script_try_open@gml_Object_par_ladder_Create_0",
//		(PVOID*)&gml_Script_spawn_ladder
//	);
//
//	if (!AurieSuccess(status))
//	{
//		g_ModuleInterface->Print(CM_LIGHTGREEN, "[ChutesAndLadders] - Failed to get script (gml_Script_spawn_ladder@DungeonRunner@DungeonRunner)!");
//	}
//
//	status = MmCreateHook(
//		g_ArSelfModule,
//		"gml_Script_try_open@gml_Object_par_ladder_Create_0",
//		gml_Script_spawn_ladder->m_Functions->m_ScriptFunction,
//		GmlScriptSpawnLadderCallback,
//		nullptr
//	);
//
//	if (!AurieSuccess(status))
//	{
//		g_ModuleInterface->Print(CM_LIGHTGREEN, "[ChutesAndLadders] - Failed to hook script (gml_Script_spawn_ladder@DungeonRunner@DungeonRunner)!");
//	}
//}

//void CreateHookGmlScriptWriteRockToLocation(AurieStatus& status)
//{
//	CScript* gml_Script_write_rock_to_location = nullptr;
//	status = g_ModuleInterface->GetNamedRoutinePointer(
//		"gml_Script_write_rock_to_location",
//		(PVOID*)&gml_Script_write_rock_to_location
//	);
//
//	if (!AurieSuccess(status))
//	{
//		g_ModuleInterface->Print(CM_LIGHTGREEN, "[ChutesAndLadders] - Failed to get script (gml_Script_write_rock_to_location)!");
//	}
//
//	status = MmCreateHook(
//		g_ArSelfModule,
//		"gml_Script_write_rock_to_location",
//		gml_Script_write_rock_to_location->m_Functions->m_ScriptFunction,
//		GmlScriptWriteRockToLocationCallback,
//		nullptr
//	);
//
//	if (!AurieSuccess(status))
//	{
//		g_ModuleInterface->Print(CM_LIGHTGREEN, "[ChutesAndLadders] - Failed to hook script (gml_Script_write_rock_to_location)!");
//	}
//}

void CreateHookGmlScriptTryLocationIdToString(AurieStatus& status)
{
	CScript* gml_script_try_location_id_to_string = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_try_location_id_to_string",
		(PVOID*)&gml_script_try_location_id_to_string
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[ChutesAndLadders] - Failed to get script (gml_Script_try_location_id_to_string)!");
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
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[ChutesAndLadders] - Failed to hook script (gml_Script_try_location_id_to_string)!");
	}
}

//void CreateHookGmlScriptLoadGame(AurieStatus& status)
//{
//	CScript* gml_script_load_game = nullptr;
//	status = g_ModuleInterface->GetNamedRoutinePointer(
//		"gml_Script_load_game",
//		(PVOID*)&gml_script_load_game
//	);
//
//	if (!AurieSuccess(status))
//	{
//		g_ModuleInterface->Print(CM_LIGHTGREEN, "[ChutesAndLadders] - Failed to get script (gml_Script_load_game)!");
//	}
//
//	status = MmCreateHook(
//		g_ArSelfModule,
//		"gml_Script_load_game",
//		gml_script_load_game->m_Functions->m_ScriptFunction,
//		GmlScriptLoadGameCallback,
//		nullptr
//	);
//
//	if (!AurieSuccess(status))
//	{
//		g_ModuleInterface->Print(CM_LIGHTGREEN, "[SaveAnywhere] - Failed to hook script (gml_Script_load_game)!");
//	}
//}

//void CreateHookGmlScriptShowRoomTitle(AurieStatus& status)
//{
//	CScript* gml_script_modify_stamina = nullptr;
//	status = g_ModuleInterface->GetNamedRoutinePointer(
//		"gml_Script_show_room_title",
//		(PVOID*)&gml_script_modify_stamina
//	);
//
//	if (!AurieSuccess(status))
//	{
//		g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Failed to get script (gml_Script_show_room_title)!");
//	}
//
//	status = MmCreateHook(
//		g_ArSelfModule,
//		"gml_Script_show_room_title",
//		gml_script_modify_stamina->m_Functions->m_ScriptFunction,
//		GmlScriptShowRoomTitleCallback,
//		nullptr
//	);
//
//	if (!AurieSuccess(status))
//	{
//		g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Failed to hook script (gml_Script_show_room_title)!");
//	}
//}

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
	
	//CreateHookGmlScriptSpawnLadder(status);
	//if (!AurieSuccess(status))
	//{
	//	g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Exiting due to failure on start!");
	//	return status;
	//}

	//CreateHookGmlScriptWriteRockToLocation(status);
	//if (!AurieSuccess(status))
	//{
	//	g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Exiting due to failure on start!");
	//	return status;
	//}

	CreateHookGmlScriptTryLocationIdToString(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Exiting due to failure on start!");
		return status;
	}

	//CreateHookGmlScriptLoadGame(status);
	//if (!AurieSuccess(status))
	//{
	//	g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Exiting due to failure on start!");
	//	return status;
	//}

	//CreateHookGmlScriptShowRoomTitle(status);
	//if (!AurieSuccess(status))
	//{
	//	g_ModuleInterface->Print(CM_LIGHTRED, "[ChutesAndLadders] - Exiting due to failure on start!");
	//	return status;
	//}

	// gml_Script_try_location_id_to_string
	g_ModuleInterface->Print(CM_LIGHTGREEN, "[ChutesAndLadders] - Plugin started!");
	return AURIE_SUCCESS;
}