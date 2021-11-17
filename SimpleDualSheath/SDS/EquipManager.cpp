#include "pch.h"

#include "EquipManager.h"

#include "Util/Common.h"

#include <ext/GameCommon.h>

namespace SDS
{
	using namespace Util::Common;

	bool EquipExtensions::CheckDualWield(Actor* a_actor)
	{
		auto race = Game::GetActorRace(a_actor);

		if (!race)
		{
			return false;
		}

		if ((race->data.raceFlags & TESRace::kRace_CanDualWield) != TESRace::kRace_CanDualWield)
		{
			return false;
		}

		if (auto extraCombatStyle = a_actor->extraData.Get<ExtraCombatStyle>(); extraCombatStyle)
		{
			if (auto cs = extraCombatStyle->combatStyle)
			{
				return (cs->flags & TESCombatStyle::kFlag_AllowDualWielding) == TESCombatStyle::kFlag_AllowDualWielding;
			}
		}

		if (auto baseForm = a_actor->baseForm; baseForm)
		{
			if (auto npc = baseForm->As<TESNPC>(); npc)
			{
				if (auto cs = npc->combatStyle)
				{
					return (cs->flags & TESCombatStyle::kFlag_AllowDualWielding) == TESCombatStyle::kFlag_AllowDualWielding;
				}
			}
		}

		return false;
	}

	bool EquipExtensions::ActorQualifiesForEquip(Actor* a_actor)
	{
		return a_actor != *g_thePlayer && CheckDualWield(a_actor) && !a_actor->IsDead();
	}

	EquipCandidateCollector::EquipCandidateCollector(
		TESObjectWEAP* a_ignore) :
		m_ignore(a_ignore)
	{
	}

	bool EquipCandidateCollector::Accept(TESContainer::ConfigEntry* entry)
	{
		if (entry && entry->form && entry->count > 0)
		{
			Process(entry->form);
		}

		return true;
	}

	bool EquipCandidateCollector::Accept(InventoryEntryData* a_entryData)
	{
		if (a_entryData && a_entryData->type && a_entryData->countDelta > 0)
		{
			Process(a_entryData->type);
		}

		return true;
	}

	void EquipCandidateCollector::Process(TESForm* a_item)
	{
		if (a_item == m_ignore)
		{
			return;
		}

		if (!a_item->Has3D())
		{
			return;
		}

		auto weapon = a_item->As<TESObjectWEAP>();
		if (!weapon)
		{
			return;
		}

		if (!CanEquipEitherHand(weapon))
		{
			return;
		}

		m_results.emplace(weapon);
	}

	enum class EquipItemResult
	{
		kEquipped,
		kFailed,
		kSlotInUse,
		kNotEnoughItems
	};

	// modified EquipItemEx (SKSE)
	static EquipItemResult EquipItem(
		EquipManager* a_equipManager,
		ExtraContainerChanges::Data* a_containerData,
		Actor* a_actor,
		TESObjectWEAP* a_item)
	{
		auto targetEquipSlot = GetLeftHandSlot();
		if (!targetEquipSlot)
		{
			return EquipItemResult::kFailed;
		}

		auto entryData = a_containerData->CreateEquipEntryData(a_item);
		if (!entryData)
		{
			return EquipItemResult::kFailed;
		}

		auto itemCount = entryData->countDelta;

		bool isTargetSlotInUse = false;

		bool hasItemMinCount = itemCount > 0;

		BaseExtraList* rightEquipList = nullptr;
		BaseExtraList* leftEquipList = nullptr;
		BaseExtraList* enchantList = nullptr;

		if (hasItemMinCount)
		{
			entryData->GetExtraWornBaseLists(&rightEquipList, &leftEquipList);

			if (leftEquipList && rightEquipList)
			{
				isTargetSlotInUse = true;
			}
			else if (rightEquipList)
			{
				isTargetSlotInUse = false;
			}
			else if (leftEquipList)
			{
				isTargetSlotInUse = true;
			}
			else
			{
				isTargetSlotInUse = false;
				enchantList = entryData->extendDataList->GetNthItem(0);
			}
		}

		entryData->Delete();

		if (isTargetSlotInUse)
		{
			return EquipItemResult::kSlotInUse;
		}

		if (rightEquipList || leftEquipList)
		{
			hasItemMinCount = itemCount > 1;
		}

		if (!hasItemMinCount)
		{
			return EquipItemResult::kNotEnoughItems;
		}

		a_equipManager->EquipItem(a_actor, a_item, enchantList, 1, targetEquipSlot, false, false, false, nullptr);

		return EquipItemResult::kEquipped;
	}

	void EquipExtensions::QueueEvaluateEquip(TESObjectREFR* a_actor) const
	{
		ITaskPool::QueueActorTask(a_actor, [this](Actor* a_actor, Game::ActorHandle) {
			EvaluateEquip(a_actor);
		});
	}

	void EquipExtensions::EvaluateEquip(Actor* a_actor) const
	{
		auto equipManager = EquipManager::GetSingleton();
		if (!equipManager)
		{
			return;
		}

		auto pm = a_actor->processManager;
		if (!pm)
		{
			return;
		}

		auto left = pm->equippedObject[ActorProcessManager::kEquippedHand_Left];
		if (left)
		{
			return;
		}

		auto right = pm->equippedObject[ActorProcessManager::kEquippedHand_Right];
		if (!right)
		{
			return;
		}

		auto weaponRight = right->As<TESObjectWEAP>();
		if (!weaponRight)
		{
			return;
		}

		if (!CanEquipEitherHand(weaponRight))
		{
			return;
		}

		/*auto weaponState = (a_actor->actorState.flags08 >> 5 & 7);
        if (weaponState != 0 && weaponState != 3) {
            return;
        }*/

		auto containerChanges = a_actor->extraData.Get<ExtraContainerChanges>();
		if (!containerChanges)
		{
			return;
		}

		auto containerData = containerChanges->data;
		if (!containerData)
		{
			return;
		}

		auto res = EquipItem(equipManager, containerData, a_actor, weaponRight);
		switch (res)
		{
		case EquipItemResult::kSlotInUse:
		case EquipItemResult::kEquipped:
			return;
		}

		EquipCandidateCollector collector(weaponRight);

		if (auto baseForm = a_actor->baseForm; baseForm)
		{
			if (auto npc = baseForm->As<TESNPC>(); npc)
			{
				npc->container.Visit(collector);
			}
		}

		if (containerData->objList)
		{
			containerData->objList->Visit(collector);
		}

		if (collector.m_results.empty())
		{
			return;
		}

		std::vector<std::pair<std::uint32_t, TESObjectWEAP*>> sortedWeapons;
		sortedWeapons.reserve(collector.m_results.size());

		for (const auto& e : collector.m_results)
		{
			auto damage = static_cast<std::uint32_t>(e->damage.attackDamage);

			auto it = std::lower_bound(
				sortedWeapons.cbegin(),
				sortedWeapons.cend(),
				damage,
				[](auto& a_data, auto a_value) {
					return a_data.first > a_value;
				});

			sortedWeapons.emplace(it, damage, e);
		}

		for (const auto& e : sortedWeapons)
		{
			auto res = EquipItem(equipManager, containerData, a_actor, e.second);
			switch (res)
			{
			case EquipItemResult::kSlotInUse:
			case EquipItemResult::kEquipped:
				return;
			}
		}
	}

	auto EquipExtensions::ReceiveEvent(TESContainerChangedEvent* a_evn, EventDispatcher<TESContainerChangedEvent>*)
		-> EventResult
	{
		if (a_evn)
		{
			if (auto actor = a_evn->newContainer.As<Actor>(); actor)
			{
				if (auto weapon = a_evn->baseObj.As<TESObjectWEAP>(); weapon)
				{
					if (ActorQualifiesForEquip(actor))
					{
						if (CanEquipEitherHand(weapon))
						{
							QueueEvaluateEquip(actor);
						}
					}
				}
			}
		}

		return kEvent_Continue;
	}

}