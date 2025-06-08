#include "hooks.h"

namespace Hooks {

	void LoadHooks()
	{
		MapMenuEx::InstallMapMenuHook();
		ItemManip::InstallAddItemHook();
		ItemManip::InstallPickupHook();
		ItemManip::InstallDropObjectHook();
		CompassHook::InstallCompassHook();
	}
#pragma region MapHook
	void MapMenuEx::InstallMapMenuHook()
	{
		REL::Relocation<std::uintptr_t> vTable(RE::VTABLE_MapMenu[0]);
		_OpenMap = vTable.write_vfunc(0x4, &MapOpen);
		REX::INFO("installed map open hook");
	}

	RE::UI_MESSAGE_RESULTS MapMenuEx::MapOpen(RE::UIMessage& a_message)
	{
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		const auto& settings = Config::GameSettings::GetSingleton();
		if (a_message.type == RE::UI_MESSAGE_TYPE::kShow && !settings->bypass_map_checks.GetValue()) {
			if (!shouldOpenMap(player)) {
				REX::INFO("restrict map hook");
				showRestrictionMessage();
				return RE::UI_MESSAGE_RESULTS::kIgnore;
			}
			else {
				RE::TESObjectMISC* curr_map = GetCurrentMapItem(player);
				const auto& forms = Config::FormCache::GetSingleton();
				if (curr_map != forms->map_indestructible && !HasIndestructibleMap(player)) {
					DurabilityTracker::GetSingleton()->DamageItem(curr_map, player, 1);
				}
			}
		}		
		return _OpenMap(this, a_message);
	}

	bool MapMenuEx::hasAtLeastOneMapItem(RE::PlayerCharacter* player)
	{
		const auto& forms = Config::FormCache::GetSingleton();
		if (player->GetItemCount(forms->map_new) > 0 ||
			player->GetItemCount(forms->map_damaged) > 0 ||
			player->GetItemCount(forms->map_indestructible) > 0) {
			return true;
		}

		const auto& inv = player->GetInventory();
		for (const auto& [object, pair] : inv) {
			if (object && object->HasKeywordByEditorID(Config::IngameFormConstants::kMapIndestructibleKeyword)) {
				return true;
			}
		}
		return false;
	}

	bool MapMenuEx::shouldOpenMap(RE::PlayerCharacter* player)
	{
		return hasAtLeastOneMapItem(player) || Config::GameSettings::GetSingleton()->bypass_map_checks.GetValue();
	}

	bool MapMenuEx::HasIndestructibleMap(RE::PlayerCharacter* player)
	{
		const auto& forms = Config::FormCache::GetSingleton();
		if (player->GetItemCount(forms->map_indestructible) > 0) {
			return true;
		}
		const auto& inv = player->GetInventory();
		for (const auto& [object, pair] : inv) {
			if (object && object->HasKeywordByEditorID(Config::IngameFormConstants::kMapIndestructibleKeyword)) {
				return true;
			}
		}
		return false;
	}

	RE::TESObjectMISC* MapMenuEx::GetCurrentMapItem(RE::PlayerCharacter* player)
	{
		const auto& forms = Config::FormCache::GetSingleton();
		if (player->GetItemCount(forms->map_new) > 0) {
			return forms->map_new;
		}
		if (player->GetItemCount(forms->map_damaged) > 0) {
			return forms->map_damaged;
		}
		if (player->GetItemCount(forms->map_indestructible) > 0) {
			return forms->map_indestructible;
		}
		return nullptr;
	}

	void MapMenuEx::showRestrictionMessage()
	{
		const auto& settings = Config::GameSettings::GetSingleton();
		std::string text = settings->restriction_message.GetValue();
		RE::DebugNotification(text.c_str(), nullptr, true);
	}

#pragma endregion MapHook
#pragma region DurabiltyTracker

	void DurabilityTracker::AddItemToPool(RE::TESBoundObject* a_item, uint32_t a_durabilityAmount, uint32_t a_itemCount)
	{
		if (!a_item)
			return;
		durability_pool[a_item] += a_durabilityAmount * a_itemCount;
		REX::INFO("added {} of {} to the durability map. total durability is now: {}", a_itemCount, a_item->GetName(), durability_pool[a_item]);
	}

	void DurabilityTracker::DamageItem(RE::TESBoundObject* a_item, RE::PlayerCharacter* a_player, uint32_t a_damageAmount)
	{
		if (!a_item)
			return;

		uint32_t item_dur = durability_amounts.contains(a_item) ? durability_amounts[a_item] : 0;
		REX::INFO("item durability is {}", item_dur);

		if (durability_pool.contains(a_item)) {
			durability_pool[a_item] -= a_damageAmount;

			if (durability_pool[a_item] <= 0) {
				a_player->RemoveItem(a_item, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
				durability_pool.erase(a_item);
				if (a_item == Config::FormCache::GetSingleton()->compass_new) {
					CompassHook::GetSingleton()->UpdateCompassState();
				}
			}
			else {
				uint32_t itemCount = a_player->GetItemCount(a_item);
				uint32_t max_possible_durability = itemCount * item_dur;
				if (durability_pool[a_item] < (max_possible_durability - item_dur)) {
					a_player->RemoveItem(a_item, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
					if (a_item == Config::FormCache::GetSingleton()->compass_new) {
						CompassHook::GetSingleton()->UpdateCompassState();
					}
				}
			}
			REX::INFO("Damaged {} by {}, new durability: {}", a_item->GetName(), a_damageAmount, durability_pool[a_item]);
		}
	}

	void DurabilityTracker::RemoveItemFromPool(RE::TESBoundObject* a_item, uint32_t a_durabilityAmount, uint32_t a_itemCount)
	{
		if (!a_item)
			return;
		if (durability_pool.contains(a_item)) {			
			uint32_t removal_amount = a_durabilityAmount * a_itemCount;
			if (removal_amount >= durability_pool[a_item]) {
				durability_pool.erase(a_item);
			}
			else {
				durability_pool[a_item] -= removal_amount;
			}				
		}		
	}

	uint32_t DurabilityTracker::GetRemainingDurability(RE::TESBoundObject* a_item) const
	{
		return durability_pool.contains(a_item) ? durability_pool.at(a_item) : 0;
	}

	bool DurabilityTracker::IsItemBroken(RE::TESBoundObject* a_item) const
	{
		return durability_pool.contains(a_item) ? durability_pool.at(a_item) <= 0 : true;
	}	

	void DurabilityTracker::PopulateMapFromInventory(RE::PlayerCharacter* player)
	{
		const auto& inv = player->GetInventory();
		for (auto& item : inv) {
			if (tracked_items.contains(item.first)) {
				if (!durability_pool.contains(item.first)) {
					AddItemToPool(item.first, durability_amounts[item.first], (uint32_t)item.second.first);
				}
			}
		}
	}

	void DurabilityTracker::GenerateDurabilityAmounts()
	{
		const auto& forms = Config::FormCache::GetSingleton();
		const auto& settings = Config::GameSettings::GetSingleton();
		durability_amounts = {
			{forms->map_new, settings->durability_map_normal},
			{forms->map_damaged, settings->durability_map_damaged},
			{forms->compass_new, settings->compass_durability}
		};
		tracked_items = {
			forms->map_new, forms->map_damaged, forms->compass_new
		};
	}

#pragma endregion DurabilityTracker
#pragma region ItemManip
	void ItemManip::InstallAddItemHook()
	{
		REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] };
		_AddObjectToContainer = PlayerCharacterVtbl.write_vfunc(0x5A, OnItemAdded);
		REX::INFO("Installed OnItemAdded Hook");
	}

	void ItemManip::InstallPickupHook()
	{
		REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] };
		_PickUpObject = PlayerCharacterVtbl.write_vfunc(0xCC, PickUpObject);
		REX::INFO("Installed PickUpObject Hook");
	}

	void ItemManip::InstallDropObjectHook()
	{
		REL::Relocation<std::uintptr_t> PlayerVtbl{ RE::VTABLE_PlayerCharacter[0] };
		_DropObject = PlayerVtbl.write_vfunc(0x0CB, DropObject);
		REX::INFO("Installed DropObject Hook");
	}

	void ItemManip::PickUpObject(RE::Actor* a_this, RE::TESObjectREFR* a_object, uint32_t a_count, bool a_arg3, bool a_playSound)
	{
		_PickUpObject(a_this, a_object, a_count, a_arg3, a_playSound);
		const auto& tracker = DurabilityTracker::GetSingleton();
		if (tracker->tracked_items.contains(a_object->GetBaseObject())) {
			REX::INFO("picked up {} of {}", a_count, a_object->GetBaseObject()->GetName());
			tracker->AddItemToPool(a_object->GetBaseObject(), tracker->durability_amounts[a_object->GetBaseObject()], a_count);
			CompassHook::GetSingleton()->UpdateCompassState();
		}
	}

	void ItemManip::OnItemAdded(RE::Actor* a_this, RE::TESBoundObject* a_object, RE::ExtraDataList* a_extraList, int32_t a_count, RE::TESObjectREFR* a_fromRefr)
	{
		_AddObjectToContainer(a_this, a_object, a_extraList, a_count, a_fromRefr);
		const auto& tracker = DurabilityTracker::GetSingleton();
		if (tracker->tracked_items.contains(a_object)) {
			REX::INFO("added {} of {}", a_count, a_object->GetName());
			tracker->AddItemToPool(a_object, tracker->durability_amounts[a_object], a_count);
			CompassHook::GetSingleton()->UpdateCompassState();
		}
	}

	RE::ObjectRefHandle ItemManip::DropObject(RE::PlayerCharacter* player, const RE::TESBoundObject* a_object, RE::ExtraDataList* a_extraList, std::int32_t a_count, const RE::NiPoint3* a_dropLoc, const RE::NiPoint3* a_rotate)
	{
		auto handle = _DropObject(player, a_object, a_extraList, a_count, a_dropLoc, a_rotate);
		if (a_object) {			
			const auto& tracker = DurabilityTracker::GetSingleton();
			auto obj = const_cast<RE::TESBoundObject*>(a_object);
			if (tracker->tracked_items.contains(obj)) {
				REX::INFO("removed {} of {}", a_count, a_object->GetName());
				tracker->RemoveItemFromPool(obj, tracker->durability_amounts[obj], a_count);
				CompassHook::GetSingleton()->UpdateCompassState();
			}			
		}
		return handle;
	}
#pragma endregion ItemManip
#pragma region CompassHook
	void CompassHook::InstallCompassHook()
	{
		REL::Relocation<std::uintptr_t> vTable(RE::VTABLE_Compass[0]);
		_UpdateComp = vTable.write_vfunc(0x1, &Update);
		REX::INFO("Compass Update installed");
	}

	bool CompassHook::GetCompassState() const
	{
		return state_show_compass;
	}

	void CompassHook::SetCompassState(bool b_show)
	{
		state_show_compass = b_show;
	}

	void CompassHook::UpdateCompassState()
	{
		state_show_compass = ShouldShowCompass();
		bool shouldShow = ShouldShowCompass();
		SetCompassState(shouldShow);
	}

	void CompassHook::ForceShowCompass()
	{
		SetCompassState(true);
	}

	void CompassHook::Update(RE::HUDObject* a_this)
	{
		Timer timer;
		if (a_this) {
			auto MovieView = a_this->view.get();
			if (MovieView) {
				RE::GFxValue compassHolder; 
				a_this->root.GetMember("CompassShoutMeterHolder", &compassHolder);
				if (compassHolder.IsDisplayObject()) {
					RE::GFxValue actual_compass;
					compassHolder.GetMember("Compass", &actual_compass);
					RE::GFxValue::DisplayInfo displayInfo;
					actual_compass.GetDisplayInfo(std::addressof(displayInfo));
					if (!CompassHook::GetSingleton()->GetCompassState()) {                    
						displayInfo.SetAlpha(0.0f);
						actual_compass.SetDisplayInfo(displayInfo);						
					}
					else {
						displayInfo.SetAlpha(100.0f);
						actual_compass.SetDisplayInfo(displayInfo);
						CompassHook::GetSingleton()->DoDamageCompass();
					}  
				}
			}
			return _UpdateComp(a_this);
		}
	}

	bool CompassHook::HasCompassItem() const
	{
		const auto& forms = Config::FormCache::GetSingleton();
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (player->GetItemCount(forms->compass_new) > 0 ||	player->GetItemCount(forms->compass_indestructible) > 0){
			REX::INFO("has either compass or indestructible compass");
			return true;
		}

		const auto& inv = player->GetInventory();
		for (const auto& [object, pair] : inv) {
			if (object && object->HasKeywordByEditorID(Config::IngameFormConstants::kCompassIndestructibleKeyword)) {
				REX::INFO("has item with compass keyword, it is: {}", object->GetName());
				return true;
			}
		}
		return false;
	}

	bool CompassHook::ShouldShowCompass() const
	{
		REX::INFO("should show compass called");
		bool should_show = HasCompassItem() || Config::GameSettings::GetSingleton()->bypass_compass_checks.GetValue();
		if (Config::GameSettings::GetSingleton()->skills_of_the_wild_active) {
			auto forms = Config::FormCache::GetSingleton();
			if (forms->sotw_perk->value == 0 || forms->sotw_cheat->value == 0) {
				should_show = false;
			}
		}
		CompassHook::GetSingleton()->SetCompassState(should_show);
		return should_show;
	}

	bool CompassHook::HasIndestructibleCompass(RE::PlayerCharacter* a_player) const
	{
		const auto& forms = Config::FormCache::GetSingleton();
		if (a_player->GetItemCount(forms->compass_indestructible) > 0){
			return true;
		}

		const auto& inv = a_player->GetInventory();
		for (const auto& [object, pair] : inv) {
			if (object && object->HasKeywordByEditorID(Config::IngameFormConstants::kCompassIndestructibleKeyword)) {
				return true;
			}
		}

		return false;
	}

	RE::TESObjectMISC* CompassHook::GetCompassFromInventory(RE::PlayerCharacter* a_player) const
	{
		const auto& forms = Config::FormCache::GetSingleton();
		if (a_player->GetItemCount(forms->compass_new) > 0) {
			return forms->compass_new;
		}
		if (a_player->GetItemCount(forms->compass_indestructible) > 0) {
			return forms->compass_indestructible;
		}
		return nullptr;
	}

	void CompassHook::ShowCompassBreakMessage()
	{
		const auto& settings = Config::GameSettings::GetSingleton();
		if (settings->show_compass_break.GetValue()) {
			std::string text = settings->compass_break_message.GetValue();
			RE::DebugNotification(text.c_str(), nullptr, true);
		}		
	}

	void CompassHook::DoDamageCompass()
	{
		
		const auto& settings = Config::GameSettings::GetSingleton();
		const auto& forms = Config::FormCache::GetSingleton();
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

		if (settings->bypass_compass_checks.GetValue() || !settings->enable_compass_damage.GetValue()) {
			compass_timer.Start();
			return;
		}

		if (!GetCompassState()) {
			compass_timer.Stop();
			return;
		}

		if (!compass_timer.IsRunning())
			compass_timer.Start();

		if (compass_timer.ElapsedSeconds() >= settings->damage_ticks_compass.GetValue()) {
			DurabilityTracker::GetSingleton()->DamageItem(forms->compass_new, player, 1);
			compass_timer.Reset();
		}
	}

#pragma endregion CompassHook

}