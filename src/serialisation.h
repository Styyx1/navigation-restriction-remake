#pragma once
#include "hooks.h"
#include "settings.h"
// Credits: https://github.com/colinswrath/BladeAndBlunt/blob/main/include/Serialization.h
namespace Serialisation
{
	static constexpr std::uint32_t SerializationVersion = 2;
	static constexpr std::uint32_t ID = 'SNRM'; //Styyx Navigation Restrictions Mod
	static constexpr std::uint32_t SerializationType = 'SMAC'; // Styyx Map and Compass


	inline void SaveCallback(SKSE::SerializationInterface* a_skse)
	{
		if (!a_skse->OpenRecord(SerializationType, SerializationVersion)) {
			REX::CRITICAL("Failed to open navigation restriction record");
			return;
		}
		else {
			const auto& damageData = Hooks::DurabilityTracker::GetSingleton();
			const auto& forms = Config::FormCache::GetSingleton();
			uint32_t map_normal_dur_saved = damageData->durability_pool[forms->map_new];
			uint32_t map_damaged_dur_save = damageData->durability_pool[forms->map_damaged];
			uint32_t comp_new_dur_saved = damageData->durability_pool[forms->compass_new];

			if (!a_skse->WriteRecordData(map_normal_dur_saved)) {
				REX::CRITICAL("Failed to save durability of map");
				return;
			}
			if (!a_skse->WriteRecordData(map_damaged_dur_save)) {
				REX::CRITICAL("Failed to save durability of damaged map");
				return;
			}
			if (!a_skse->WriteRecordData(comp_new_dur_saved)) {
				REX::CRITICAL("Failed to save durability of compass");
				return;
			}

			else {
				REX::INFO("Saved total Map Normal durability: {}", map_normal_dur_saved);
				REX::INFO("Saved total Map Damaged durability: {}", map_damaged_dur_save);
				REX::INFO("Saved total Compass durability: {}", comp_new_dur_saved);
			}

		}
	}

	inline void LoadCallback(SKSE::SerializationInterface* a_skse)
	{
		std::uint32_t type;
		std::uint32_t version;
		std::uint32_t length;
		a_skse->GetNextRecordInfo(type, version, length);

		if (type != SerializationType) {
			return;
		}

		if (version != SerializationVersion) {
			REX::CRITICAL("Unable to load data");
			return;
		}

		uint32_t map_normal_dur_loaded;
		uint32_t map_damaged_dur_loaded;
		uint32_t comp_new_dur_loaded;

		if (!a_skse->ReadRecordData(map_normal_dur_loaded)) {
			REX::CRITICAL("Failed to load normal map durability");
			return;
		}
		if (!a_skse->ReadRecordData(map_damaged_dur_loaded)) {
			REX::CRITICAL("Failed to load damaged map durability");
			return;
		}
		if (!a_skse->ReadRecordData(comp_new_dur_loaded)) {
			REX::CRITICAL("Failed to load compass durability");
			return;
		}
		else {
			const auto& damageData = Hooks::DurabilityTracker::GetSingleton();
			const auto& forms = Config::FormCache::GetSingleton();
			damageData->durability_pool[forms->map_new] = map_normal_dur_loaded;
			damageData->durability_pool[forms->map_damaged] = map_damaged_dur_loaded;
			damageData->durability_pool[forms->compass_new] = comp_new_dur_loaded;

			REX::INFO("loaded Map Normal durabilty: {}", map_normal_dur_loaded);
			REX::INFO("loaded Map Damaged durabilty: {}", map_damaged_dur_loaded);
			REX::INFO("loaded Compass durabilty: {}", comp_new_dur_loaded);
		}
	}

	inline void RevertCallback([[maybe_unused]] SKSE::SerializationInterface* a_skse)
	{
		const auto& damageData = Hooks::DurabilityTracker::GetSingleton();
		const auto& forms = Config::FormCache::GetSingleton();

		damageData->durability_pool[forms->map_new] = 0;
		damageData->durability_pool[forms->map_damaged] = 0;
		damageData->durability_pool[forms->compass_new] = 0;
	}
}
