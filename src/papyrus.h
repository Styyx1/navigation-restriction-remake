#pragma once
#include "settings.h"
#include "Hooks.h"


namespace Papyrus
{
	using VM = RE::BSScript::Internal::VirtualMachine;
	using StackID = RE::VMStackID;

	bool Bind(VM* a_vm);

	namespace Functions
	{

		enum
		{
			kVersion = 2,
		};

		std::int32_t GetVersion(VM*, StackID, RE::StaticFunctionTag*);
		void UpdateNRSettings(VM*, StackID, RE::StaticFunctionTag*);
		std::int32_t GetRemainingDurability(VM*, StackID, RE::StaticFunctionTag*, RE::TESBoundObject* a_item);
		void ForceShowCompass(VM*, StackID, RE::StaticFunctionTag*);

		void Bind(VM& a_vm);
	}
}
