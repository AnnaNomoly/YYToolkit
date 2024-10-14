#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static const int FULL_RESTORE_SPELL_ID = 0;
static const int GROWTH_SPELL_ID = 1;
static const int SUMMON_RAIN_SPELL_ID = 2;
static const double CHARM_SPELL_COST = 2.0;

static YYTKInterface* g_ModuleInterface = nullptr;
static double ari_current_mana = -1.0;
static int pinned_spell = -1;
static double heart_multiplier = 1.50;
static bool charm_spell_active = false;
static bool room_loaded = false;
static bool modify_mana = false;

bool GameIsPaused()
{
	CInstance* global_instance = nullptr;
	g_ModuleInterface->GetGlobalInstance(&global_instance);
	RValue paused = global_instance->at("__pause_status");
	return paused.m_i64 > 0;
}

RValue& GmlScriptSetPinnedSpell(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if(Arguments[0]->m_Kind == VALUE_INT64)
		pinned_spell = Arguments[0]->m_i64;
	if (Arguments[0]->m_Kind == VALUE_REAL)
		pinned_spell = Arguments[0]->m_Real;

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_set_pinned_spell@Ari@Ari"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptCanCastSpell(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_can_cast_spell"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	if (!charm_spell_active && pinned_spell == FULL_RESTORE_SPELL_ID && ari_current_mana >= CHARM_SPELL_COST)
	{
		Result.m_Real = 1.0;
	}

	return Result;
}

RValue& GmlScriptCastSpell(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if (pinned_spell == FULL_RESTORE_SPELL_ID)
	{
		modify_mana = true;
		charm_spell_active = true;
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[CharmSpell] - Charm spell cast. Friendship points gained for the day are increased!");
		return Result;
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_cast_spell"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptAddHeartPointsCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if (charm_spell_active)
	{
		double original_value = Arguments[0]->m_Real;
		double modified_value = ceil(heart_multiplier * Arguments[0]->m_Real);
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[CharmSpell] - Increased heart point gain! [Original: %f] ==> [Boosted: %f]", original_value, modified_value);

		Arguments[0]->m_Real = ceil(heart_multiplier * Arguments[0]->m_Real);
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_add_heart_points@Npc@Npc"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptSetupMainScreenCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	ari_current_mana = -1.0;
	pinned_spell = -1;
	charm_spell_active = false;
	room_loaded = false;
	modify_mana = false;

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

RValue& GmlScriptNewDayCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	charm_spell_active = false;
	room_loaded = false;
	modify_mana = false;

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_try_end_of_day_status_to_string"));
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
	room_loaded = true;
	
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


RValue& GmlScriptGetManaCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_get_mana@Ari@Ari"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	if (room_loaded && Result.m_Kind != VALUE_UNSET)
	{
		ari_current_mana = Result.m_Real;
	}

	return Result;
}

RValue& GmlScriptModifyManaCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if (modify_mana)
	{
		Arguments[0]->m_Real = (CHARM_SPELL_COST * -1.0);
		modify_mana = false;
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_modify_mana@Ari@Ari"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptOnDrawGuiCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_on_draw_gui@Display@Display"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	if (room_loaded && !GameIsPaused() && charm_spell_active)
	{
		RValue hunger_bar_sprite_index = g_ModuleInterface->CallBuiltin(
			"asset_get_index", {
				"spr_ui_hud_charm_spell_icon"
			}
		);

		g_ModuleInterface->CallBuiltin(
			"draw_sprite", {
				hunger_bar_sprite_index, 1, 10, 200
			}
		);
	}

	return Result;
}

void CreateHookGmlScriptSetPinnedSpell(AurieStatus& status)
{
	CScript* gml_script_set_pinned_spell = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_set_pinned_spell@Ari@Ari",
		(PVOID*)&gml_script_set_pinned_spell
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to get script (gml_Script_set_pinned_spell@Ari@Ari)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_set_pinned_spell@Ari@Ari",
		gml_script_set_pinned_spell->m_Functions->m_ScriptFunction,
		GmlScriptSetPinnedSpell,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to hook script (gml_Script_set_pinned_spell@Ari@Ari)!");
	}
}

void CreateHookGmlScriptCanCastSpell(AurieStatus& status)
{
	CScript* gml_script_can_cast_spell = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_can_cast_spell",
		(PVOID*)&gml_script_can_cast_spell
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to get script (gml_Script_can_cast_spell)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_can_cast_spell",
		gml_script_can_cast_spell->m_Functions->m_ScriptFunction,
		GmlScriptCanCastSpell,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to hook script (gml_Script_can_cast_spell)!");
	}
}

void CreateHookGmlScriptCastSpell(AurieStatus& status)
{
	CScript* gml_script_cast_spell = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_cast_spell",
		(PVOID*)&gml_script_cast_spell
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to get script (gml_Script_cast_spell)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_cast_spell",
		gml_script_cast_spell->m_Functions->m_ScriptFunction,
		GmlScriptCastSpell,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to hook script (gml_Script_enchantern_state_to_string)!");
	}
}

void CreateHookGmlScriptAddHeartPoints(AurieStatus& status)
{
	CScript* gml_script_add_heart_points = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_add_heart_points@Npc@Npc",
		(PVOID*)&gml_script_add_heart_points
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to get script (gml_Script_add_heart_points@Npc@Npc)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_add_heart_points@Npc@Npc",
		gml_script_add_heart_points->m_Functions->m_ScriptFunction,
		GmlScriptAddHeartPointsCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to hook script (gml_Script_add_heart_points@Npc@Npc)!");
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
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to get script (gml_Script_setup_main_screen@TitleMenu@TitleMenu)!");
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
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to hook script (gml_Script_setup_main_screen@TitleMenu@TitleMenu)!");
	}
}

void CreateHookGmlScriptNewDay(AurieStatus& status)
{
	CScript* gml_script_new_day = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_try_end_of_day_status_to_string",
		(PVOID*)&gml_script_new_day
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to get script (gml_Script_new_day)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_try_end_of_day_status_to_string",
		gml_script_new_day->m_Functions->m_ScriptFunction,
		GmlScriptNewDayCallback,
		nullptr
	);


	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to hook script (gml_Script_new_day)!");
	}
}

void CreateHookGmlScriptShowRoomTitle(AurieStatus& status)
{
	CScript* gml_script_show_room_title = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_show_room_title",
		(PVOID*)&gml_script_show_room_title
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to get script (gml_Script_show_room_title)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_show_room_title",
		gml_script_show_room_title->m_Functions->m_ScriptFunction,
		GmlScriptShowRoomTitleCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to hook script (gml_Script_show_room_title)!");
	}
}

void CreateHookGmlScriptGetMana(AurieStatus& status)
{
	CScript* gml_script_get_mana = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_get_mana@Ari@Ari",
		(PVOID*)&gml_script_get_mana
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to get script (gml_Script_get_mana@Ari@Ari)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_get_mana@Ari@Ari",
		gml_script_get_mana->m_Functions->m_ScriptFunction,
		GmlScriptGetManaCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to hook script (gml_Script_get_mana@Ari@Ari)!");
	}
}

void CreateHookGmlScriptModifyMana(AurieStatus& status)
{
	CScript* gml_script_get_mana = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_modify_mana@Ari@Ari",
		(PVOID*)&gml_script_get_mana
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to get script (gml_Script_modify_mana@Ari@Ari)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_modify_mana@Ari@Ari",
		gml_script_get_mana->m_Functions->m_ScriptFunction,
		GmlScriptModifyManaCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to hook script (gml_Script_modify_mana@Ari@Ari)!");
	}
}

void CreateHookGmlScriptOnDrawGui(AurieStatus& status)
{
	CScript* gml_script_on_draw_gui = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_on_draw_gui@Display@Display",
		(PVOID*)&gml_script_on_draw_gui
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to get script (gml_Script_on_draw_gui@Display@Display)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_on_draw_gui@Display@Display",
		gml_script_on_draw_gui->m_Functions->m_ScriptFunction,
		GmlScriptOnDrawGuiCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Failed to hook script (gml_Script_on_draw_gui@Display@Display)!");
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

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[CharmSpell] - Plugin starting...");

	CreateHookGmlScriptSetPinnedSpell(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptCanCastSpell(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptCastSpell(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptAddHeartPoints(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptSetupMainScreen(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptNewDay(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptShowRoomTitle(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptGetMana(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptModifyMana(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptOnDrawGui(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[CharmSpell] - Exiting due to failure on start!");
		return status;
	}

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[CharmSpell] - Plugin started!");
	return AURIE_SUCCESS;
}