#include "serialisation.h"
#include "papyrus.h"

void InitListener(SKSE::MessagingInterface::Message* a_msg) {
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kInputLoaded:
		break;
	case SKSE::MessagingInterface::kDataLoaded:
		Config::FormCache::GetSingleton()->LoadForms();
		Hooks::DurabilityTracker::GetSingleton()->GenerateDurabilityAmounts();		
		break;	
	case SKSE::MessagingInterface::kNewGame:
		Hooks::DurabilityTracker::GetSingleton()->PopulateMapFromInventory(RE::PlayerCharacter::GetSingleton());
		Hooks::CompassHook::GetSingleton()->UpdateCompassState();
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
		Hooks::DurabilityTracker::GetSingleton()->PopulateMapFromInventory(RE::PlayerCharacter::GetSingleton());
		Hooks::CompassHook::GetSingleton()->UpdateCompassState();
		break;
	default: 
		break;
	}
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	Config::GameSettings::GetSingleton()->Load();
	SKSE::GetMessagingInterface()->RegisterListener(InitListener);
	SKSE::GetPapyrusInterface()->Register(Papyrus::Bind);
	Hooks::LoadHooks();

	if (const auto& serialization = SKSE::GetSerializationInterface()) {
		serialization->SetUniqueID(Serialisation::ID);
		serialization->SetSaveCallback(&Serialisation::SaveCallback);
		serialization->SetLoadCallback(&Serialisation::LoadCallback);
		serialization->SetRevertCallback(&Serialisation::RevertCallback);
	}

	return true;
}
