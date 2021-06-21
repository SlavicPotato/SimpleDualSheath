#include "pch.h"

#include "Controller.h"

#include "Util/Common.h"
#include "Util/Task.h"
#include "Util/Node.h"

#include <ext/GameCommon.h>

#include <ext/VFT.h>

namespace SDS
{
    using namespace Util::Common;
    using namespace Util::Task;
    using namespace Util::Node;
    using namespace Data;

    Controller::Controller(
        const Config& a_conf)
        :
        m_conf(a_conf)
    {
    }

    void Controller::InitializeData()
    {
        m_strings = std::make_shared<StringHolder>();
        m_data = std::make_unique<WeaponData>();

        m_data->Create(TESObjectWEAP::GameData::kType_OneHandSword, StringHolder::NINODE_SWORD, StringHolder::NINODE_SWORD_LEFT, m_conf.m_sword);
        m_data->Create(TESObjectWEAP::GameData::kType_OneHandAxe, StringHolder::NINODE_AXE, StringHolder::NINODE_AXE_LEFT, m_conf.m_axe);
        m_data->Create(TESObjectWEAP::GameData::kType_OneHandMace, StringHolder::NINODE_MACE, StringHolder::NINODE_MACE_LEFT, m_conf.m_mace);
        m_data->Create(TESObjectWEAP::GameData::kType_OneHandDagger, StringHolder::NINODE_DAGGER, StringHolder::NINODE_DAGGER_LEFT, m_conf.m_dagger);
        m_data->Create(TESObjectWEAP::GameData::kType_Staff, StringHolder::NINODE_STAFF, StringHolder::NINODE_STAFF_LEFT, m_conf.m_staff);
        m_data->Create(TESObjectWEAP::GameData::kType_TwoHandSword, StringHolder::NINODE_WEAPON_BACK, StringHolder::NINODE_SWORD_ON_BACK_LEFT, m_conf.m_2hSword);
        m_data->Create(TESObjectWEAP::GameData::kType_TwoHandAxe, StringHolder::NINODE_WEAPON_BACK, StringHolder::NINODE_AXE_ON_BACK_LEFT, m_conf.m_2hAxe);

        if (!m_conf.m_shield.m_sheathNode.empty()) {
            m_strings->m_shieldSheathNode.Set(m_conf.m_shield.m_sheathNode.c_str());
        }

#ifdef _SDS_UNUSED
        m_nodeOverride = std::make_unique<NodeOverride>(m_strings);
#endif 

    }

    bool Controller::GetParentNodes(
        const Data::Weapon* a_entry,
        NiNode* a_root,
        bool a_left,
        NiPointer<NiNode>& a_sheathedNode,
        NiPointer<NiNode>& a_drawnNode) const
    {
        auto nodea = a_entry->GetNode(a_root, a_left);
        if (!nodea) {
            return false;
        }

        auto nodeb = FindNode(a_root, a_left ? m_strings->m_shield : m_strings->m_weapon);
        if (!nodeb) {
            return false;
        }

        a_sheathedNode = nodea;
        a_drawnNode = nodeb;

        return true;
    }

    bool Controller::GetIsDrawn(
        Actor* a_actor,
        DrawnState a_state)
    {
        switch (a_state)
        {
        case DrawnState::Drawn:
            return true;
        case DrawnState::Sheathed:
            return false;
        default:
            return a_actor->actorState.IsWeaponDrawn();
        };
    }

    std::uint32_t Controller::GetShieldBipedObject(
        Actor* a_actor)
    {
        if (auto baseForm = a_actor->baseForm; baseForm)
        {
            if (baseForm->formType == TESNPC::kTypeID)
            {
                auto npc = static_cast<TESNPC*>(baseForm);
                if (auto race = npc->race.race; race)
                {
                    return race->data.shieldObject;
                }
            }
        }

        return 0xFFFFFFFF;
    }

    void Controller::ProcessEquippedWeapon(
        Actor* a_actor,
        const NiRootNodes& a_roots,
        TESObjectWEAP* a_weapon,
        bool a_drawn,
        bool a_left) const
    {
        auto entry = m_data->Get(a_actor, a_weapon, a_left);
        if (!entry) {
            return;
        }

        if (!CanEquipEitherHand(a_weapon)) {
            return;
        }

        char buf[MAX_PATH];
        a_weapon->GetNodeName(buf);

        BSFixedString weaponNodeName(buf);

        for (std::size_t i = 0; i < std::size(a_roots.m_nodes); i++)
        {
            auto& root = a_roots.m_nodes[i];

            if (!root) {
                continue;
            }

            if (i == 1 && (entry->m_flags & Data::Flags::kFirstPerson) != Data::Flags::kFirstPerson) {
                continue;
            }

            NiPointer<NiNode> sheathedNode, drawnNode;
            if (!GetParentNodes(entry, root, a_left, sheathedNode, drawnNode)) {
                continue;
            }

            auto& sourceNode = a_drawn ? sheathedNode : drawnNode;
            auto& targetNode = a_drawn ? drawnNode : sheathedNode;

            if (NiPointer weaponNode = GetNiObject(sourceNode, weaponNodeName); weaponNode)
            {
                /*gLog.Message("[%.8X] [%s] [WEAP: %X | left: %d] Attaching [%s] from [%s] to [%s] [root: %s] [1p:%zu] [%p]",
                    a_actor->formID.get(),
                    a_actor->GetReferenceName(),
                    a_weapon->formID.get(),
                    a_left,
                    weaponNode->m_name.c_str(),
                    sourceNode->m_name.c_str(),
                    targetNode->m_name.c_str(),
                    root->m_name.c_str(),
                    i,
                    weaponNode.get());*/

                AttachToNode(weaponNode, targetNode);
                ClearCull(weaponNode);
            }
            else if (NiPointer weaponNode = GetNiObject(targetNode, weaponNodeName); weaponNode)
            {
                // if the weapon node is attached to target and invisible just remove the cull flag

                /*gLog.Message("[%.8X] [%s] [WEAP: %X | left: %d] Clearing cull flag [%s] [root: %s] [1p:%zu] [%p]",
                    a_actor->formID.get(),
                    a_actor->GetReferenceName(),
                    a_weapon->formID.get(),
                    a_left,
                    weaponNode->m_name.c_str(),
                    root->m_name.c_str(),
                    i,
                    weaponNode.get());*/

                ClearCull(weaponNode);
            }
        }
    }

    void Controller::ProcessWeaponDrawnChange(
        Actor* a_actor,
        bool a_drawn) const
    {
        //IScopedLock lock(m_lock);

        auto pm = a_actor->processManager;
        if (!pm) {
            return;
        }

        NiRootNodes roots(a_actor);
        roots.GetNPCRoots(m_strings->m_npcroot);

        auto form = pm->equippedObject[ActorProcessManager::kEquippedHand_Left];
        if (form)
        {
            if (form->IsWeapon())
            {
                ProcessEquippedWeapon(a_actor, roots, static_cast<TESObjectWEAP*>(form), a_drawn, true);
            }
            else if (form->IsArmor())
            {
                if (m_conf.m_shield.IsEnabled())
                {
                    ProcessEquippedShield(a_actor, roots, a_drawn);
                }
            }
        }

        form = pm->equippedObject[ActorProcessManager::kEquippedHand_Right];
        if (form)
        {
            if (form->IsWeapon())
            {
                ProcessEquippedWeapon(a_actor, roots, static_cast<TESObjectWEAP*>(form), a_drawn, false);
            }
        }
    }

    void Controller::QueueProcessWeaponDrawnChange(
        TESObjectREFR* a_actor,
        DrawnState a_drawnState) const
    {
        QueueActorTask(a_actor, [this, a_drawnState](Actor* a_actor)
            {
                ProcessWeaponDrawnChange(a_actor, GetIsDrawn(a_actor, a_drawnState));
            }
        );
    }

    bool Controller::IsShieldEnabled(Actor* a_actor) const
    {
        if (a_actor == *g_thePlayer)
        {
            if ((m_conf.m_shield.m_flags & Flags::kPlayer) == Flags::kPlayer) {
                return true;
            }
        }
        else
        {
            if ((m_conf.m_shield.m_flags & Flags::kNPC) == Flags::kNPC) {
                return true;
            }
        }

        return false;
    }

    bool Controller::ShouldBlockShieldHide(Actor* a_actor) const
    {
        if (a_actor == *g_thePlayer)
        {
            if ((m_conf.m_shieldHideFlags & Flags::kPlayer) != Flags::kPlayer) {
                return false;
            }
        }
        else
        {
            if ((m_conf.m_shieldHideFlags & Flags::kNPC) != Flags::kNPC) {
                return false;
            }
        }

        if ((m_conf.m_shieldHideFlags & Flags::kMountOnly) == Flags::kMountOnly)
        {
            return (a_actor->flags2 & Actor::kFlags_kGettingOnOffMount) == Actor::kFlags_kGettingOnOffMount ||
                a_actor->IsOnMount();
        }

        return true;
    }

    void Controller::ProcessEquippedShield(
        Actor* a_actor,
        const NiRootNodes& a_roots,
        bool a_drawn) const
    {
        if (!IsShieldEnabled(a_actor)) {
            return;
        }

        auto bipedObject = GetShieldBipedObject(a_actor);

        if (bipedObject >= 42u) {
            return;
        }

        for (std::size_t i = 0; i < std::size(a_roots.m_nodes); i++)
        {
            auto& root = a_roots.m_nodes[i];

            if (!root) {
                continue;
            }

            bool firstPerson = i == 1;

            if (firstPerson && !m_conf.m_shield.FirstPerson()) {
                continue;
            }

            auto bipedModel = a_actor->GetBiped(firstPerson);
            if (!bipedModel) {
                continue;
            }

            auto bipedData = bipedModel->bipedData;
            if (!bipedData) {
                continue;
            }

            auto& data = bipedData->unk10[bipedObject];

            auto form = data.armor;

            if (!form || !form->IsArmor()) {
                continue;
            }

            if (!static_cast<TESObjectARMO*>(form)->IsShield()) {
                continue;
            }

            NiPointer armorNode = data.object;
            if (!armorNode) {
                continue;
            }

            auto& targetNodeName = a_drawn ? m_strings->m_shield : m_strings->m_shieldSheathNode;

            NiPointer targetNode = FindNode(root, targetNodeName);
            if (!targetNode) {
                continue;
            }

            AttachToNode(armorNode, targetNode);
        }
    }

    NiNode* Controller::FindObjectNPCRoot(
        TESObjectREFR* a_actor,
        NiAVObject* a_object,
        bool a_no1p) const
    {
        NiRootNodes roots(a_actor, a_no1p);

        auto root = a_object->GetAsNiNode();
        if (!root) {
            root = a_object->m_parent;
        }

        NiNode* found(nullptr);

        while (root)
        {
            if (roots.MatchesAny(root)) {
                found = root;
            }

            root = root->m_parent;
        }

        if (!found) {
            return nullptr;
        }

        if (auto npcroot = FindNode(found, m_strings->m_npcroot); npcroot) {
            found = npcroot;
        }

        return found;
    }

    NiNode* Controller::GetScbAttachmentNode(
        Actor* a_actor,
        TESForm* a_form,
        NiNode* a_attachmentNode) const
    {
        auto weap = static_cast<TESObjectWEAP*>(a_form);

        auto entry = m_data->Get(a_actor, weap, true);
        if (!entry) {
            return nullptr;
        }

        if ((entry->m_flags & Data::Flags::kSwap) == Data::Flags::kSwap) {
            return nullptr;
        }

        bool no1p = (entry->m_flags & Data::Flags::kFirstPerson) != Data::Flags::kFirstPerson;

        NiPointer root = FindObjectNPCRoot(a_actor, a_attachmentNode, no1p);
        if (!root) {
            return nullptr;
        }

        return entry->GetNode(root, true);
    }

    const BSFixedString* Controller::GetWeaponAttachmentNodeName(
        Actor* a_actor, 
        TESForm* a_form,
        bool a_firstPerson, 
        bool a_left) const
    {
        if (!a_form->IsWeapon()) {
            return nullptr;
        }

        auto weapon = static_cast<TESObjectWEAP*>(a_form);

        auto entry = m_data->Get(a_actor, weapon, a_left);
        if (!entry) {
            return nullptr;
        }
        
        if (a_firstPerson && (entry->m_flags & Data::Flags::kFirstPerson) != Data::Flags::kFirstPerson) {
            return nullptr;
        }

        return std::addressof(entry->GetNodeName(a_left));
    }
        
    const BSFixedString* Controller::GetShieldAttachmentNodeName(
        Actor* a_actor, 
        TESForm* a_form,
        bool a_firstPerson) const
    {
        if (!a_form->IsArmor()) {
            return nullptr;
        }

        auto armor = static_cast<TESObjectARMO*>(a_form);
        if (!armor->IsShield()) {
            return nullptr;
        }

        if (!IsShieldEnabled(a_actor)) {
            return nullptr;
        }

        if (a_actor->actorState.IsWeaponDrawn()) {
            return nullptr;
        }

        if (a_firstPerson && (m_conf.m_shield.m_flags & Data::Flags::kFirstPerson) != Data::Flags::kFirstPerson) {
            return nullptr;
        }

        NiPointer root = a_actor->GetNiRootNode(a_firstPerson);
        if (!root) {
            return nullptr;
        }

        NiPointer npcRoot = FindNode(root, m_strings->m_npcroot);
        if (!npcRoot) {
            return nullptr;
        }

        return std::addressof(m_strings->m_shieldSheathNode);
    }

    void Controller::OnActorLoad(TESObjectREFR* a_actor)
    {
        QueueActorTask(a_actor, [this](Actor* a_actor)
            {

#ifdef _SDS_UNUSED

                m_nodeOverride->ApplyNodeOverrides(a_actor);
#endif
                ProcessWeaponDrawnChange(a_actor, a_actor->actorState.IsWeaponDrawn());

                if (m_conf.m_npcEquipLeft && ActorQualifiesForEquip(a_actor))
                {
                    EvaluateEquip(a_actor);
                }
            }
        );
    }
#ifdef _SDS_UNUSED

    void Controller::OnNiNodeUpdate(TESObjectREFR* a_actor)
    {
        QueueActorTask(a_actor, [this](Actor* a_actor)
            {
                m_nodeOverride->ApplyNodeOverrides(a_actor);
            }
        );
    }
#endif

    auto Controller::ReceiveEvent(TESObjectLoadedEvent* a_evn, EventDispatcher<TESObjectLoadedEvent>*)
        -> EventResult
    {
        if (a_evn && a_evn->loaded)
        {
            auto form = a_evn->formId.Lookup();
            if (form && form->formType == Actor::kTypeID)
            {
                OnActorLoad(static_cast<TESObjectREFR*>(form));
            }
        }

        return kEvent_Continue;
    }

    auto Controller::ReceiveEvent(TESInitScriptEvent* a_evn, EventDispatcher<TESInitScriptEvent>*)
        -> EventResult
    {
        if (a_evn)
        {
            OnActorLoad(a_evn->reference);
        }

        return kEvent_Continue;
    }

    void Controller::OnWeaponEquip(Actor* a_actor, TESObjectWEAP* a_weapon)
    {
        if (!m_conf.m_npcEquipLeft) {
            return;
        }

        if (!ActorQualifiesForEquip(a_actor)) {
            return;
        }

        auto pm = a_actor->processManager;
        if (!pm) {
            return;
        }

        if (a_weapon != pm->equippedObject[ActorProcessManager::kEquippedHand_Right]) {
            return;
        }

        if (CanEquipEitherHand(a_weapon)) {
            QueueEvaluateEquip(a_actor);
        }
    }

    auto Controller::ReceiveEvent(TESEquipEvent* a_evn, EventDispatcher<TESEquipEvent>*)
        -> EventResult
    {
        if (a_evn && a_evn->equipped && a_evn->actor)
        {
            if (a_evn->actor->formType == Actor::kTypeID)
            {
                if (auto form = a_evn->baseObject.Lookup(); form)
                {
                    if (form->IsWeapon())
                    {
                        auto actor = static_cast<Actor*>(a_evn->actor.get());
                        OnWeaponEquip(actor, static_cast<TESObjectWEAP*>(form));
                    }
                }
            }
        }

        return kEvent_Continue;
    }

#ifdef _SDS_UNUSED
    auto Controller::ReceiveEvent(SKSENiNodeUpdateEvent* a_evn, EventDispatcher<SKSENiNodeUpdateEvent>*)
        -> EventResult
    {
        if (a_evn) {
            OnNiNodeUpdate(a_evn->reference);
        }

        return kEvent_Continue;
    }
#endif

    auto Controller::ReceiveEvent(SKSEActionEvent* a_evn, EventDispatcher<SKSEActionEvent>*)
        -> EventResult
    {
        if (a_evn)
        {
            switch (a_evn->type)
            {
            case SKSEActionEvent::kType_EndDraw:
                QueueProcessWeaponDrawnChange(a_evn->actor, DrawnState::Drawn);
                break;
            case SKSEActionEvent::kType_EndSheathe:
                QueueProcessWeaponDrawnChange(a_evn->actor, DrawnState::Sheathed);
                break;
            }
        }

        return kEvent_Continue;
    }

    void Controller::EvaluateDrawnStateOnNearbyActors()
    {
        auto task = new TaskFunctor([this]
            {
                auto player = *g_thePlayer;
                if (player) {
                    ProcessWeaponDrawnChange(player, player->actorState.IsWeaponDrawn());
                }

                auto pl = Game::ProcessLists::GetSingleton();
                if (!pl) {
                    return;
                }

                for (auto handle : pl->highActorHandles)
                {
                    NiPointer<TESObjectREFR> ref;

                    if (!handle.LookupREFR(ref)) {
                        continue;
                    }

                    if (ref->formType != Actor::kTypeID) {
                        continue;
                    }

                    auto actor = static_cast<Actor*>(ref.get());
                    ProcessWeaponDrawnChange(actor, actor->actorState.IsWeaponDrawn());
                }
            }
        );

        ISKSE::GetSingleton().GetInterface<SKSETaskInterface>()->AddTask(task);
    }

    void Controller::Receive(const Events::OnSetEquipSlot& a_evn)
    {
        auto player = *g_thePlayer;
        if (!player || !player->loadedState) {
            return;
        }

        EvaluateDrawnStateOnNearbyActors();
    }


}


