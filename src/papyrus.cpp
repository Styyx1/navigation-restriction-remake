#include "Papyrus.h"
#include "Hooks.h"

namespace Papyrus
{
	bool Bind(VM* a_vm)
	{
		if (!a_vm) {
			REX::CRITICAL("couldn't get VM State"sv);
			return false;
		}

		REX::INFO("{:*^30}", "FUNCTIONS"sv);

		Functions::Bind(*a_vm);

		return true;
	}

	namespace Functions
	{
		std::int32_t GetVersion(VM*, StackID, RE::StaticFunctionTag*)
		{
			return kVersion;
		}

		void UpdateNRSettings(VM*, StackID, RE::StaticFunctionTag*)
		{
			Config::GameSettings::GetSingleton()->Load();
			Hooks::DurabilityTracker::GetSingleton()->GenerateDurabilityAmounts();
		}

		std::int32_t GetRemainingDurability(VM*, StackID, RE::StaticFunctionTag*, RE::TESBoundObject* a_item)
		{
			if (!a_item) {
				REX::CRITICAL("can not call function on NONE object");
			}
			return Hooks::DurabilityTracker::GetSingleton()->GetRemainingDurability(a_item);
		}
		void ForceShowCompass(VM*, StackID, RE::StaticFunctionTag*) {
			Hooks::CompassHook::GetSingleton()->ForceShowCompass();
		}

		void Bind(VM& a_vm)
		{
			constexpr auto script = "NavigationRestrictionUtil"sv;

			a_vm.RegisterFunction("GetVersion", script, GetVersion, true);
			a_vm.RegisterFunction("UpdateNRSettings", script, UpdateNRSettings);
			a_vm.RegisterFunction("GetRemainingDurability", script, GetRemainingDurability);
			a_vm.RegisterFunction("ForceShowCompass", script, ForceShowCompass);
			REX::INFO("Registered navigation restriction functions"sv);
		}
	}	
}