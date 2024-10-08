#include <iostream>
#include <fstream>
#include <codecvt>
#include <shlobj.h>
#include <filesystem>
#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;

static int ari_x;
static int ari_y;
static int ari_room_id;
static int saved_x;
static int saved_y;
static int saved_room_id;
static bool modded_save = false;
static bool room_changed = false;
static bool wait_to_teleport_ari = false;
static bool ready_to_teleport_ari = false;
static bool wait_to_reposition_ari = false;
static bool ready_to_reposition_ari = false;
static std::string loaded_save;

std::string wstr_to_string(std::wstring wstr)
{
	std::vector<char> buf(wstr.size());
	std::use_facet<std::ctype<wchar_t>>(std::locale{}).narrow(wstr.data(), wstr.data() + wstr.size(), '?', buf.data());

	return std::string(buf.data(), buf.size());
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

	RValue x;
	g_ModuleInterface->GetBuiltin("x", self, NULL_INDEX, x);
	ari_x = x.m_Real;

	RValue y;
	g_ModuleInterface->GetBuiltin("y", self, NULL_INDEX, y);
	ari_y = y.m_Real;

	if (wait_to_teleport_ari && ready_to_teleport_ari)
	{
		CScript* gml_script_goto_location_id = nullptr;
		g_ModuleInterface->GetNamedRoutinePointer(
			"gml_Script_goto_location_id",
			(PVOID*)&gml_script_goto_location_id
		);

		RValue retval;
		RValue* location_id = new RValue(saved_room_id);
		gml_script_goto_location_id->m_Functions->m_ScriptFunction(
			self,
			other,
			retval,
			1,
			{ &location_id }
		);
		delete location_id;

		wait_to_teleport_ari = false;
		ready_to_teleport_ari = false;
		wait_to_reposition_ari = true;
	}
	else if (wait_to_reposition_ari && ready_to_reposition_ari) {
		RValue x = saved_x;
		RValue y = saved_y;
		g_ModuleInterface->SetBuiltin("x", self, NULL_INDEX, x);
		g_ModuleInterface->SetBuiltin("y", self, NULL_INDEX, y);
		wait_to_reposition_ari = false;
		ready_to_reposition_ari = false;
	}
}

RValue& GmlScriptLoadGameCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	std::string arg0_str = std::string(Arguments[0]->m_Object->at("save_path").AsString().data());
	std::size_t index = arg0_str.find_last_of("/");
	loaded_save = arg0_str.substr(index + 1);

	wchar_t* localAppDataFolder;
	if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &localAppDataFolder) == S_OK)
	{
		// Check if the SaveAnywhere mod directory exists.
		std::string mod_data_folder = wstr_to_string(localAppDataFolder) + "\\FieldsOfMistria\\mod_data\\SaveAnywhere\\";
		if (std::filesystem::exists(mod_data_folder))
		{
			// Read from the save file in mod_data.
			std::ifstream file(mod_data_folder + "\\" + loaded_save);
			if (file.good())
			{
				file >> saved_room_id >> saved_x >> saved_y;
				wait_to_teleport_ari = true;
			}
			file.close();
		}
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_load_game"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptSaveGameCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if (!modded_save)
	{
		wchar_t* localAppDataFolder;
		if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &localAppDataFolder) == S_OK)
		{
			std::string file_name = wstr_to_string(localAppDataFolder) + "\\FieldsOfMistria\\mod_data\\SaveAnywhere\\" + loaded_save;
			std::remove(file_name.c_str());
		}
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_save_game"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptHeldItemCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if (GetAsyncKeyState(VK_HOME) & 1)
	{
		wchar_t* localAppDataFolder;
		if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &localAppDataFolder) == S_OK)
		{
			// Create the mods directory if it doesn't exist.
			std::string mod_data_folder = wstr_to_string(localAppDataFolder) + "\\FieldsOfMistria\\mod_data\\";
			if (!std::filesystem::exists(mod_data_folder))
			{
				std::filesystem::create_directories(mod_data_folder);

				// Create the SaveAnywhere directory if it doesn't exist.
				std::string save_anywhere_folder = mod_data_folder + "\\SaveAnywhere";
				if (!std::filesystem::exists(save_anywhere_folder))
				{
					std::filesystem::create_directories(save_anywhere_folder);
				}
			}
			else {
				// Create the SaveAnywhere directory if it doesn't exist.
				std::string save_anywhere_folder = mod_data_folder + "\\SaveAnywhere";
				if (!std::filesystem::exists(save_anywhere_folder))
				{
					std::filesystem::create_directories(save_anywhere_folder);
				}
			}

			// Write the file in mod_data.
			std::string file_name = wstr_to_string(localAppDataFolder) + "\\FieldsOfMistria\\mod_data\\SaveAnywhere\\" + loaded_save;
			std::ofstream outfile(file_name, std::ios::out);
			if (outfile.is_open())
			{
				outfile << std::to_string(ari_room_id) + " " + std::to_string(ari_x) + " " + std::to_string(ari_y);
			}
			outfile.close();

			// Save the game.
			CScript* gml_script_save_game = nullptr;
			g_ModuleInterface->GetNamedRoutinePointer(
				"gml_Script_save_game",
				(PVOID*)&gml_script_save_game
			);

			modded_save = true;
			file_name = wstr_to_string(localAppDataFolder) + "\\FieldsOfMistria\\saves\\" + loaded_save;
			RValue retval_1;
			RValue* arg = new RValue(file_name);
			gml_script_save_game->m_Functions->m_ScriptFunction(
				Self,
				Other,
				retval_1,
				1,
				{
					&arg
				}
			);

			modded_save = false;
			delete arg;

			// Create save notification.
			CScript* gml_script_save_game_notification = nullptr;
			g_ModuleInterface->GetNamedRoutinePointer(
				"gml_Script_create_save_notification",
				(PVOID*)&gml_script_save_game_notification
			);
			
			RValue retval_2;
			gml_script_save_game_notification->m_Functions->m_ScriptFunction(
				Self,
				Other,
				retval_2,
				0,
				nullptr
			);
		}
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

RValue& GmlScriptShowRoomTitleCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	room_changed = true;
	if (wait_to_reposition_ari) {
		ready_to_reposition_ari = true;
	}
	if (wait_to_teleport_ari) {
		ready_to_teleport_ari = true;
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_show_room_title"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptTryLocationIdToStringCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if (room_changed)
	{
		ari_room_id = Arguments[0]->m_Real;
		room_changed = false;
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_try_location_id_to_string"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

void CreateHookObject(AurieStatus& status)
{
	status = g_ModuleInterface->CreateCallback(
		g_ArSelfModule,
		EVENT_OBJECT_CALL,
		ObjectCallback,
		0
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[SaveAnywhere] - Failed to hook (ObjectCallback)!");
	}
}

void CreateHookGmlScriptLoadGame(AurieStatus& status)
{
	CScript* gml_script_held_item = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_load_game",
		(PVOID*)&gml_script_held_item
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[SaveAnywhere] - Failed to get script (gml_Script_load_game)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_load_game",
		gml_script_held_item->m_Functions->m_ScriptFunction,
		GmlScriptLoadGameCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[SaveAnywhere] - Failed to hook script (gml_Script_load_game)!");
	}
}

void CreateHookGmlScriptSaveGame(AurieStatus& status)
{
	CScript* gml_script_held_item = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_save_game",
		(PVOID*)&gml_script_held_item
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[SaveAnywhere] - Failed to get script (gml_Script_create_save_notification)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_save_game",
		gml_script_held_item->m_Functions->m_ScriptFunction,
		GmlScriptSaveGameCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[SaveAnywhere] - Failed to hook script (gml_Script_create_save_notification)!");
	}
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
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[SaveAnywhere] - Failed to get script (gml_Script_held_item@Ari@Ari)!");
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
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[SaveAnywhere] - Failed to hook script (gml_Script_held_item@Ari@Ari)!");
	}
}

void CreateHookGmlScriptShowRoomTitle(AurieStatus& status)
{
	CScript* gml_script_modify_stamina = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_show_room_title",
		(PVOID*)&gml_script_modify_stamina
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[SaveAnywhere] - Failed to get script (gml_Script_show_room_title)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_show_room_title",
		gml_script_modify_stamina->m_Functions->m_ScriptFunction,
		GmlScriptShowRoomTitleCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[SaveAnywhere] - Failed to hook script (gml_Script_show_room_title)!");
	}
}

void CreateHookGmlScriptTryLocationIdToString(AurieStatus& status)
{
	CScript* gml_script_held_item = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_try_location_id_to_string",
		(PVOID*)&gml_script_held_item
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[SaveAnywhere] - Failed to get script (gml_Script_try_location_id_to_string)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_try_location_id_to_string",
		gml_script_held_item->m_Functions->m_ScriptFunction,
		GmlScriptTryLocationIdToStringCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[SaveAnywhere] - Failed to hook script (gml_Script_try_location_id_to_string)!");
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

	g_ModuleInterface->Print(CM_LIGHTYELLOW, "[SaveAnywhere] - Plugin loading...");

	CreateHookObject(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[SaveAnywhere] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptLoadGame(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[SaveAnywhere] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptSaveGame(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[SaveAnywhere] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptHeldItem(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[SaveAnywhere] - Exiting due to failure on start!");
		return status;
	}
	
	CreateHookGmlScriptShowRoomTitle(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[SaveAnywhere] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptTryLocationIdToString(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[SaveAnywhere] - Exiting due to failure on start!");
		return status;
	}

	return AURIE_SUCCESS;
}