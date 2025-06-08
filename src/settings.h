#pragma once

namespace Config {

    namespace Constants {
        inline const std::string toml_file_path1 = "Data/SKSE/Plugins/NavigationRestrictions.toml";
        inline const std::string toml_file_path2 = "Data/SKSE/Plugins/NavigationRestrictions_Custom.toml";
        inline const std::string json_file_path = "Data/SKSE/Plugins/NavigationRestrictions.json";
        inline const std::string ini_file_path1 = "Data/MCM/Config/NavigationRestrictions/settings.ini";
        inline const std::string ini_file_path2 = "Data/MCM/Settings/NavigationRestrictions.ini";
        inline const char* mod_name = "NavigationRestrictions.esp";
    };

    namespace IngameFormConstants {
        inline const char* mod_name = "NavigationRestrictions.esp";
        inline const char* skills_of_the_wild_mod = "SkillsOfTheWild.esp";
        constexpr const char* kMapIndestructibleKeyword = "MapIndestructible";
        constexpr const char* kCompassIndestructibleKeyword = "CompassIndestructible";
        inline const int map_new_item_ID = 0x800;
        inline const int map_damaged_item_ID = 0x801;
        inline const int map_indestructible_item_ID = 0x803;
        inline const int map_destroyed_item_ID = 0x802;
        inline const int compass_new_item_ID = 0x804;
        inline const int compass_indestructible_item_ID = 0x81C;
        inline const int sotw_compass_global_cheat_id = 0x863;
        inline const int sotw_compass_global_non_cheat_id = 0x958;
    }

	struct GameSettings : public REX::Singleton<GameSettings> 
	{

        bool skills_of_the_wild_active{false};

        inline static REX::INI::Bool enable_debugging{ "Logging", "bEnableDebug", false };

        inline static REX::INI::Bool enable_compass_damage{ "Settings", "bEnableCompassDamage", true };
        inline static REX::INI::Bool bypass_map_checks{ "Settings", "bBypassMapCheck", false };
        inline static REX::INI::Bool bypass_compass_checks{ "Settings", "bBypassCompassCheck", false  };
        inline static REX::INI::Bool show_compass_break{ "Settings", "bShowCompassNotif",true };
        inline static REX::INI::F32 damage_ticks_compass{ "Settings", "fDamageTickDuration", 30.0f };
        
        inline static REX::INI::I32 durability_map_normal{ "Settings", "iMapDurability", 20 };
        inline static REX::INI::I32 durability_map_damaged{ "Settings", "iDamagedMapDurability", 10 };
        inline static REX::INI::I32 compass_durability{ "Settings", "iCompassDurability", 20 };

        inline static REX::INI::Str restriction_message{ "Texts", "sMessageText", (std::string)"You need a map to see your location!"};
        inline static REX::INI::Str compass_break_message{ "Texts", "sCompassBreakMessage", (std::string)"Your compass can not guide you anymore..." };

        void Load();
        void LogSettings();
	};

    namespace Functions {
        static inline bool isSkillOfTheWildActive() 
        {
            auto dh = RE::TESDataHandler::GetSingleton();

            if (auto file = dh->LookupModByName(IngameFormConstants::skills_of_the_wild_mod); file && file->compileIndex != 0xFF) {
                REX::INFO("Skills of the wild is active");
                GameSettings::GetSingleton()->skills_of_the_wild_active = true;
            }
            return GameSettings::GetSingleton()->skills_of_the_wild_active;
        };
        static inline bool IsMainModActive() {
            auto dh = RE::TESDataHandler::GetSingleton();
            auto file = dh->LookupModByName(IngameFormConstants::mod_name); 
            if (file && file->compileIndex != 0xFF) {
                return true;
            }          
            else
            {
                REX::FAIL("{} is not loaded, please activate the .esp file of the mod to load the dll file", IngameFormConstants::mod_name);
                return false;
            }
        }
    }

    struct FormCache : public REX::Singleton<FormCache> {
        RE::TESObjectMISC* map_new{};
        RE::TESObjectMISC* map_damaged{};
        RE::TESObjectMISC* map_indestructible{};
        RE::TESObjectMISC* map_destroyed{};
        RE::TESObjectMISC* compass_new{};
        RE::TESObjectMISC* compass_indestructible{};
        RE::TESGlobal* sotw_perk{};
        RE::TESGlobal* sotw_cheat{};

        void LoadForms();
        void LogAllForms() const;
        void LogForm(RE::TESForm* form) const;
    };

}
