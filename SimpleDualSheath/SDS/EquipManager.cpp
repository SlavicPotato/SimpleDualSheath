#include "pch.h"

#include "EquipManager.h"

#include "Util/Common.h"

namespace SDS
{
    using namespace Util::Common;

    bool EquipExtensions::CheckDualWield(Actor* a_actor)
    {
        auto race = GetActorRace(a_actor);

        if (!race) {
            return false;
        }

        if ((race->data.raceFlags & TESRace::kRace_CanDualWield) != TESRace::kRace_CanDualWield) {
            return false;
        }

        auto extraCombatStyle = static_cast<ExtraCombatStyle*>(a_actor->extraData.GetByType(kExtraData_CombatStyle));
        if (extraCombatStyle)
        {
            auto cs = extraCombatStyle->combatStyle;
            if (cs) {
                return (cs->flags & TESCombatStyle::kFlag_AllowDualWielding) == TESCombatStyle::kFlag_AllowDualWielding;
            }
        }

        auto baseForm = a_actor->baseForm;

        if (baseForm && baseForm->formType == TESNPC::kTypeID)
        {
            auto cs = static_cast<TESNPC*>(baseForm)->combatStyle;
            if (cs) {
                return (cs->flags & TESCombatStyle::kFlag_AllowDualWielding) == TESCombatStyle::kFlag_AllowDualWielding;
            }
        }

        return false;
    }

    bool EquipExtensions::ActorQualifiesForEquip(Actor* a_actor)
    {
        return a_actor != *g_thePlayer && CheckDualWield(a_actor);
    }

    EquipCandidateCollector::EquipCandidateCollector(
        TESObjectWEAP* a_ignore)
        :
        m_ignore(a_ignore)
    {
        m_results.reserve(10);
    }

    bool EquipCandidateCollector::Accept(TESContainer::ConfigEntry* entry)
    {
        if (entry && entry->form) {
            Process(entry->form);
        }

        return true;
    }

    bool EquipCandidateCollector::Accept(InventoryEntryData* a_entryData)
    {
        if (a_entryData && a_entryData->type && a_entryData->countDelta > 0) {
            Process(a_entryData->type);
        }

        return true;
    }

    void EquipCandidateCollector::Process(TESForm* a_item)
    {
        if (a_item == m_ignore) {
            return;
        }

        if (!a_item->Has3D()) {
            return;
        }

        if (!a_item->IsWeapon()) {
            return;
        }

        auto weapon = static_cast<TESObjectWEAP*>(a_item);

        if (!CanEquipEitherHand(weapon)) {
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
    static EquipItemResult EquipItem(EquipManager* a_equipManager, ExtraContainerChanges::Data* a_containerData, Actor* thisActor, TESObjectWEAP* item)
    {
        auto entryData = a_containerData->CreateEquipEntryData(item);
        if (!entryData)
            return EquipItemResult::kFailed;

        auto targetEquipSlot = GetLeftHandSlot();
        if (!targetEquipSlot) {
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

        if (isTargetSlotInUse) {
            return EquipItemResult::kSlotInUse;
        }

        if (rightEquipList || leftEquipList) {
            hasItemMinCount = itemCount > 1;
        }

        if (!hasItemMinCount) {
            return EquipItemResult::kNotEnoughItems;
        }

        a_equipManager->EquipItem(thisActor, item, enchantList, 1, targetEquipSlot, false, false, false, nullptr);

        return EquipItemResult::kEquipped;
    }

    void EquipExtensions::QueueEvaluateEquip(TESObjectREFR* a_actor)
    {
        ITaskPool::QueueActorTask(a_actor, [this](Actor* a_actor)
            {
                EvaluateEquip(a_actor);
            });
    }

    void EquipExtensions::EvaluateEquip(Actor* a_actor)
    {
        auto equipManager = EquipManager::GetSingleton();
        if (!equipManager) {
            return;
        }

        auto pm = a_actor->processManager;
        if (!pm) {
            return;
        }

        auto left = pm->equippedObject[ActorProcessManager::kEquippedHand_Left];
        if (left) {
            return;
        }

        auto right = pm->equippedObject[ActorProcessManager::kEquippedHand_Right];
        if (!right) {
            return;
        }

        if (!right->IsWeapon()) {
            return;
        }

        auto weaponRight = static_cast<TESObjectWEAP*>(right);
        if (!CanEquipEitherHand(weaponRight)) {
            return;
        }

        /*auto weaponState = (a_actor->actorState.flags08 >> 5 & 7);
        if (weaponState != 0 && weaponState != 3) {
            return;
        }*/

        auto containerChanges = static_cast<ExtraContainerChanges*>(a_actor->extraData.GetByType(kExtraData_ContainerChanges));
        if (!containerChanges) {
            return;
        }

        auto containerData = containerChanges->data;

        if (!containerData || !containerData->objList) {
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
            if (auto container = RTTI<TESContainer>()(baseForm); container)
            {
                container->Visit(collector);
            }
        }

        containerData->objList->Visit(collector);

        if (collector.m_results.empty()) {
            return;
        }

        std::vector<std::pair<std::uint32_t, TESObjectWEAP*>> sortedWeapons;
        sortedWeapons.reserve(collector.m_results.size());

        for (const auto& e : collector.m_results)
        {
            auto damage = static_cast<std::uint32_t>(e->damage.GetAttackDamage());

            auto it = sortedWeapons.cbegin();
            while (it != sortedWeapons.cend())
            {
                if (damage > it->first) {
                    break;
                }

                ++it;
            }

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
            auto containerForm = a_evn->newContainer.Lookup();
            if (containerForm && containerForm->formType == Actor::kTypeID)
            {
                auto actor = static_cast<Actor*>(containerForm);
                if (ActorQualifiesForEquip(actor))
                {
                    auto baseForm = a_evn->baseObj.Lookup();
                    if (baseForm && baseForm->IsWeapon())
                    {
                        if (CanEquipEitherHand(baseForm))
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