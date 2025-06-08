#include "settings.h"

namespace Config {
	void GameSettings::Load()
	{
		auto ini = REX::INI::SettingStore::GetSingleton();
		ini->Init(Constants::ini_file_path1.c_str(), Constants::ini_file_path2.c_str());
		REX::INFO("Loading settings from: {} and {}", Constants::ini_file_path1, Constants::ini_file_path2);
		ini->Load();
		if (enable_debugging) {
			LogSettings();
		}		
	}

	void GameSettings::LogSettings()
	{
		REX::DEBUG("== Navigation Restrictions Settings ==");
		REX::DEBUG("bEnableCompassDamage: {}", enable_compass_damage.GetValue());
		REX::DEBUG("bBypassMapCheck: {}", bypass_map_checks.GetValue());
		REX::DEBUG("bBypassCompassCheck: {}", bypass_compass_checks.GetValue());
		REX::DEBUG("bShowCompassNotif: {}", show_compass_break.GetValue());
		REX::DEBUG("fDamageTickDuration: {}", damage_ticks_compass.GetValue());
		REX::DEBUG("iMapDurability: {}", durability_map_normal.GetValue());
		REX::DEBUG("iDamagedMapDurability: {}", durability_map_damaged.GetValue());
		REX::DEBUG("iCompassDurability: {}", compass_durability.GetValue());
		REX::DEBUG("sMessageText: {}", restriction_message.GetValue());
		REX::DEBUG("sCompassBreakMessage: {}", compass_break_message.GetValue());
		REX::DEBUG("======================================");
	}
	void FormCache::LoadForms()
	{
		auto dataHandler = RE::TESDataHandler::GetSingleton();
		using namespace IngameFormConstants ;

		if (Functions::IsMainModActive()) {
			map_new = dataHandler->LookupForm<RE::TESObjectMISC>(map_new_item_ID, mod_name);
			map_damaged = dataHandler->LookupForm<RE::TESObjectMISC>(map_damaged_item_ID, mod_name);
			map_destroyed = dataHandler->LookupForm<RE::TESObjectMISC>(map_destroyed_item_ID, mod_name);
			map_indestructible = dataHandler->LookupForm<RE::TESObjectMISC>(map_indestructible_item_ID, mod_name);
			compass_new  = dataHandler->LookupForm<RE::TESObjectMISC>(compass_new_item_ID, mod_name);
			compass_indestructible = dataHandler->LookupForm<RE::TESObjectMISC>(compass_indestructible_item_ID, mod_name);
			if (!map_new || !map_damaged || !map_destroyed || !map_indestructible || !compass_new || !compass_indestructible) {
				REX::FAIL("Forms couldn't be loaded, please make sure that the esp file is active and don't delete forms from it");
			}
			LogAllForms();
		}			
		if (Functions::isSkillOfTheWildActive()) {
			sotw_perk = dataHandler->LookupForm<RE::TESGlobal>(sotw_compass_global_non_cheat_id, skills_of_the_wild_mod);
			sotw_cheat = dataHandler->LookupForm<RE::TESGlobal>(sotw_compass_global_cheat_id, skills_of_the_wild_mod);
			if (!sotw_perk || !sotw_cheat || sotw_perk->GetFormType() != RE::FormType::Global || sotw_cheat->GetFormType() != RE::FormType::Global) {
				Config::GameSettings::GetSingleton()->skills_of_the_wild_active = false;
			}
		}
	}
	void FormCache::LogForm(RE::TESForm* form) const
	{	
		if (form) {
			std::string edid = editorID::get_editorID(form);
			RE::FormType ft = form->GetFormType();
			REX::INFO("{} loaded. FormType is {}", edid, RE::FormTypeToString(ft));
		}
		else
			REX::CRITICAL("Form is nullptr");
		
	}
	void FormCache::LogAllForms() const
	{
		LogForm(map_new);
		LogForm(map_damaged);
		LogForm(map_indestructible);
		LogForm(map_destroyed);
		LogForm(compass_new);
		LogForm(compass_indestructible);
		if (Functions::isSkillOfTheWildActive()) {
			LogForm(sotw_perk);
			LogForm(sotw_cheat);
		}		
	}
}

