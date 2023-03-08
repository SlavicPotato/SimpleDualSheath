#include "pch.h"

#include "EquipManager.h"

#include "Util/Common.h"

#include <ext/GameCommon.h>

namespace SDS
{
	using namespace Util::Common;

	bool EquipExtensions::CheckDualWield(Actor* a_actor)
	{
		auto race = a_actor->GetRace();

		if (!race)
		{
			return false;
		}

		if (!race->data.raceFlags.test(TESRace::Flag::kCanDualWield))
		{
			return false;
		}

		if (auto extraCombatStyle = a_actor->extraData.Get<ExtraCombatStyle>())
		{
			if (auto cs = extraCombatStyle->combatStyle)
			{
				return cs->csflags.test(TESCombatStyle::FLAG::kAllowDualWielding);
			}
		}

		if (auto npc = a_actor->GetActorBase())
		{
			if (auto cs = npc->combatStyle)
			{
				return cs->csflags.test(TESCombatStyle::FLAG::kAllowDualWielding);
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

	bool EquipCandidateCollector::Accept(TESContainer::Entry* entry)
	{
		if (entry && entry->form)
		{
			Process(entry->form, entry->count);
		}

		return true;
	}

	bool EquipCandidateCollector::Accept(InventoryEntryData* a_entryData)
	{
		if (a_entryData && a_entryData->type)
		{
			Process(a_entryData->type, a_entryData->countDelta);
		}

		return true;
	}

	template <class T>
	void EquipCandidateCollector::Process(
		TESForm* a_item,
		T        a_count)
	{
		if (a_item == m_ignore)
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

		m_results.try_emplace(weapon, 0).first->second += a_count;
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
		EquipManager*                a_equipManager,
		ExtraContainerChanges::Data* a_containerData,
		Actor*                       a_actor,
		TESObjectWEAP*               a_item)
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
		BaseExtraList* leftEquipList  = nullptr;
		BaseExtraList* enchantList    = nullptr;

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
				if (entryData->extraLists && !entryData->extraLists->empty())
				{
					enchantList = entryData->extraLists->front();
				}
			}
		}

		delete entryData;

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
		ITaskPool::QueueLoadedActorTask(
			a_actor,
			[this](Actor* a_actor, Game::ActorHandle) {
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

		if (auto npc = a_actor->GetActorBase())
		{
			npc->Visit(collector);
		}

		if (containerData->objList)
		{
			for (auto& e : *containerData->objList)
			{
				if (e)
				{
					collector.Accept(e);
				}
			}
		}

		if (collector.m_results.empty())
		{
			return;
		}

		stl::vector<std::pair<std::uint32_t, TESObjectWEAP*>> sortedWeapons;
		sortedWeapons.reserve(collector.m_results.size());

		for (const auto& e : collector.m_results)
		{
			if (e.second <= 0)
			{
				continue;
			}

			auto damage = static_cast<std::uint32_t>(e.first->attackDamage);

			auto it = std::lower_bound(
				sortedWeapons.cbegin(),
				sortedWeapons.cend(),
				damage,
				[](auto& a_data, auto a_value) {
					return a_data.first > a_value;
				});

			sortedWeapons.emplace(it, damage, e.first);
		}

		for (const auto& e : sortedWeapons)
		{
			auto r = EquipItem(equipManager, containerData, a_actor, e.second);
			switch (r)
			{
			case EquipItemResult::kSlotInUse:
			case EquipItemResult::kEquipped:
				return;
			}
		}
	}

	auto EquipExtensions::ReceiveEvent(
		const TESContainerChangedEvent* a_evn,
		BSTEventSource<TESContainerChangedEvent>*)
		-> EventResult
	{
		if (a_evn)
		{
			if (auto actor = a_evn->newContainer.As<Actor>())
			{
				if (ActorQualifiesForEquip(actor))
				{
					if (auto weapon = a_evn->baseObj.As<TESObjectWEAP>())
					{
						if (CanEquipEitherHand(weapon))
						{
							QueueEvaluateEquip(actor);
						}
					}
				}
			}
		}

		return EventResult::kContinue;
	}

}