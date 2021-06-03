#include "pch.h"

#include "Controller.h"

#include "Util/Common.h"
#include "Util/Task.h"
#include "Util/Node.h"

#include <ext/GameCommon.h>

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
        m_strings = std::make_unique<StringHolder>();
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

    bool Controller::GetIsDrawn(Actor* a_actor, DrawnState a_state)
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
                /*gLog.Message("[%.8X] [%s] [WEAP: %X | left: %d] Attaching [%s] from [%s] to [%s] [root: %s] [1p:%zu]",
                    a_actor->formID.get(),
                    a_actor->GetReferenceName(),
                    a_weapon->formID.get(),
                    a_left,
                    weaponNode->m_name,
                    sourceNode->m_name,
                    targetNode->m_name,
                    root->m_name,
                    i);*/

                AttachToNode(weaponNode, targetNode, (entry->m_flags & Flags::kUpdateNodeOnAttach) == Flags::kUpdateNodeOnAttach);
                ClearCull(weaponNode);
            }
            else if (NiPointer weaponNode = GetNiObject(targetNode, weaponNodeName); weaponNode)
            {
                // if the weapon node is attached to target and invisible just remove the cull flag

                /*gLog.Message("[%.8X] [%s] [WEAP: %X | left: %d] Clearing cull flag [%s] [root: %s] [1p:%zu]",
                    a_actor->formID.get(),
                    a_actor->GetReferenceName(),
                    a_weapon->formID.get(),
                    a_left,
                    weaponNode->m_name,
                    root->m_name,
                    i);*/

                ClearCull(weaponNode);
            }
        }
    }

    void Controller::ProcessWeaponDrawnChange(
        Actor* a_actor,
        bool a_drawn) const
    {
        auto pm = a_actor->processManager;
        if (!pm) {
            return;
        }

        NiRootNodes roots(a_actor);
        roots.GetNPCRoots(m_strings->m_npcroot);

#ifdef _SDS_UNUSED
        ApplyNodeOverrides(roots);
#endif

        auto form = pm->equippedObject[ActorProcessManager::kEquippedHand_Left];
        if (form)
        {
            if (form->IsWeapon())
            {
                ProcessEquippedWeapon(a_actor, roots, static_cast<TESObjectWEAP*>(form), a_drawn, true);
            }
            else if (form->IsArmor())
            {
                if ((m_conf.m_shield.m_flags & Flags::kEnabled) != Flags::kNone)
                {
                    auto armor = static_cast<TESObjectARMO*>(form);
                    if (armor->IsShield())
                    {
                        ProcessEquippedShield(a_actor, roots, armor, a_drawn);
                    }
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

    bool Controller::IsShieldAllowed(Actor* a_actor) const
    {
        if (a_actor == *g_thePlayer)
        {
            if ((m_conf.m_shield.m_flags & Flags::kPlayer) != Flags::kPlayer) {
                return false;
            }
        }
        else
        {
            if ((m_conf.m_shield.m_flags & Flags::kNPC) != Flags::kNPC) {
                return false;
            }
        }

        return true;
    }

    void Controller::ProcessEquippedShield(
        Actor* a_actor,
        const NiRootNodes& a_roots,
        TESObjectARMO* a_armor,
        bool a_drawn) const
    {
        if (!IsShieldAllowed(a_actor)) {
            return;
        }

        for (std::size_t i = 0; i < std::size(a_roots.m_nodes); i++)
        {
            auto& root = a_roots.m_nodes[i];

            if (!root) {
                continue;
            }

            if (i == 1 && (m_conf.m_shield.m_flags & Data::Flags::kFirstPerson) != Data::Flags::kFirstPerson) {
                continue;
            }

            NiPointer sheathedNode = FindNode(root, m_strings->m_shieldSheathNode);
            if (!sheathedNode) {
                continue;
            }

            NiPointer drawnNode = FindNode(root, m_strings->m_shield);
            if (!drawnNode) {
                continue;
            }

            auto& sourceNode = a_drawn ? sheathedNode : drawnNode;
            auto& targetNode = a_drawn ? drawnNode : sheathedNode;

            using size_type = decltype(a_armor->armorAddons.count);

            for (size_type i = 0; i < a_armor->armorAddons.count; i++)
            {
                TESObjectARMA* arma(nullptr);

                if (!a_armor->armorAddons.GetNthItem(i, arma)) {
                    continue;
                }

                if (!arma || !arma->isValidRace(a_actor->race)) {
                    continue;
                }

                char buf[MAX_PATH];
                arma->GetNodeName(buf, a_actor, a_armor, -1.0f);

                if (NiPointer armorNode = GetNiObject(sourceNode, buf); armorNode)
                {
                    AttachToNode(armorNode, targetNode, (m_conf.m_shield.m_flags & Flags::kUpdateNodeOnAttach) == Flags::kUpdateNodeOnAttach);
                    //ClearCull(armorNode);
                }
                /*else if (NiPointer armorNode = GetNiObject(targetNode, buf); armorNode)
                {
                    ClearCull(armorNode);
                }*/
            }
        }
    }

    void Controller::DoProcessEquippedShield(
        Actor* a_actor,
        DrawnState a_drawnState) const
    {
        auto pm = a_actor->processManager;
        if (!pm) {
            return;
        }

        auto form = pm->equippedObject[ActorProcessManager::kEquippedHand_Left];
        if (!form) {
            return;
        }

        if (form->formType != TESObjectARMO::kTypeID) {
            return;
        }

        auto armor = static_cast<TESObjectARMO*>(form);

        if (armor->IsShield())
        {
            NiRootNodes roots(a_actor);
            roots.GetNPCRoots(m_strings->m_npcroot);

            ProcessEquippedShield(a_actor, roots, armor, GetIsDrawn(a_actor, a_drawnState));
        }
    }

    void Controller::SetShieldGeometryCull(
        Actor* a_actor,
        DrawnState a_drawnState) const
    {
        auto pm = a_actor->processManager;
        if (!pm) {
            return;
        }

        auto form = pm->equippedObject[ActorProcessManager::kEquippedHand_Left];
        if (!form) {
            return;
        }

        if (form->formType != TESObjectARMO::kTypeID) {
            return;
        }

        auto armor = static_cast<TESObjectARMO*>(form);

        if (!armor->IsShield()) {
            return;
        }

        NiRootNodes roots(a_actor);
        roots.GetNPCRoots(m_strings->m_npcroot);

        if (!IsShieldAllowed(a_actor)) {
            return;
        }

        bool drawn = GetIsDrawn(a_actor, a_drawnState);

        for (std::size_t i = 0; i < std::size(roots.m_nodes); i++)
        {
            auto& root = roots.m_nodes[i];

            if (!root) {
                continue;
            }

            if (i == 1 && (m_conf.m_shield.m_flags & Data::Flags::kFirstPerson) != Data::Flags::kFirstPerson) {
                continue;
            }

            NiPointer sheathedNode = FindNode(root, m_strings->m_shieldSheathNode);
            if (!sheathedNode) {
                continue;
            }

            NiPointer drawnNode = FindNode(root, m_strings->m_shield);
            if (!drawnNode) {
                continue;
            }

            auto& sourceNode = drawn ? sheathedNode : drawnNode;
            auto& targetNode = drawn ? drawnNode : sheathedNode;

            using size_type = decltype(armor->armorAddons.count);

            for (size_type i = 0; i < armor->armorAddons.count; i++)
            {
                TESObjectARMA* arma(nullptr);

                if (!armor->armorAddons.GetNthItem(i, arma)) {
                    continue;
                }

                if (!arma || !arma->isValidRace(a_actor->race)) {
                    continue;
                }

                char buf[MAX_PATH];
                arma->GetNodeName(buf, a_actor, armor, -1.0f);

                if (NiPointer armorNode = GetNiObject(sourceNode, buf); armorNode)
                {
                    armorNode->m_flags |= NiAVObject::kFlag_Cull;
                }
                else if (NiPointer armorNode = GetNiObject(targetNode, buf); armorNode) {
                    armorNode->m_flags |= NiAVObject::kFlag_Cull;
                }
            }
        }
    }


    void Controller::QueueProcessEquippedShield(
        Actor* a_actor,
        DrawnState a_drawnState) const
    {
        QueueActorTask(a_actor, [this, a_drawnState](Actor* a_actor)
            {
                DoProcessEquippedShield(a_actor, a_drawnState);
            }
        );
    }

    NiNode* Controller::GetScbAttachmentNode(
        TESObjectREFR* a_actor,
        TESForm* a_form,
        NiAVObject* a_sheathNode,
        bool a_checkEquippedLeft) const
    {
        auto actor = static_cast<Actor*>(a_actor);

        if (a_checkEquippedLeft)
        {
            auto pm = actor->processManager;
            if (!pm) {
                return nullptr;
            }

            if (pm->equippedObject[ActorProcessManager::kEquippedHand_Left] != a_form) {
                return nullptr;
            }
        }

        auto weap = static_cast<TESObjectWEAP*>(a_form);

        auto entry = m_data->Get(actor, weap, true);
        if (!entry) {
            return nullptr;
        }

        if ((entry->m_flags & Data::Flags::kSwap) == Data::Flags::kSwap) {
            return nullptr;
        }

        bool no1p = (entry->m_flags & Data::Flags::kFirstPerson) != Data::Flags::kFirstPerson;

        NiRootNodes roots(a_actor, no1p);

        auto root = a_sheathNode->GetAsNiNode();
        if (!root) {
            root = a_sheathNode->m_parent;
        }

        NiNode* found(nullptr);

        while (root)
        {
            if (roots.MatchesAny(root)) {
                found = root;
            }

            root = root->m_parent;
        };

        if (!found) {
            return nullptr;
        }

        if (auto npcroot = FindNode(found, m_strings->m_npcroot); npcroot) {
            found = npcroot;
        }

        return entry->GetNode(found, true);
    }

    void Controller::OnActorLoad(TESObjectREFR* a_actor)
    {
        QueueActorTask(a_actor, [this](Actor* a_actor)
            {
                ProcessWeaponDrawnChange(a_actor, GetIsDrawn(a_actor, DrawnState::Determine));

                if (m_conf.m_npcEquipLeft && ActorQualifiesForEquip(a_actor))
                {
                    EvaluateEquip(a_actor);
                }
            }
        );
    }

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

    void Controller::OnShieldEquip(Actor* a_actor, TESObjectARMO* a_armor)
    {
        if ((m_conf.m_shield.m_flags & Data::Flags::kEnabled) == Data::Flags::kNone) {
            return;
        }

        if (a_armor->IsShield()) {
            QueueProcessEquippedShield(a_actor, DrawnState::Determine);
        }
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
            auto actor = static_cast<Actor*>(a_evn->actor.get());

            if (actor->formType == Actor::kTypeID)
            {
                auto form = a_evn->baseObject.Lookup();
                if (form)
                {
                    if (form->IsArmor())
                    {
                        OnShieldEquip(actor, static_cast<TESObjectARMO*>(form));
                    }
                    else if (form->IsWeapon())
                    {
                        OnWeaponEquip(actor, static_cast<TESObjectWEAP*>(form));
                    }
                }
            }
        }

        return kEvent_Continue;
    }

    auto Controller::ReceiveEvent(SKSENiNodeUpdateEvent* a_evn, EventDispatcher<SKSENiNodeUpdateEvent>*)
        -> EventResult
    {
        if (a_evn) {
            QueueProcessWeaponDrawnChange(a_evn->reference, DrawnState::Determine);
        }

        return kEvent_Continue;
    }


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

    // pretty sure this is useless
    auto Controller::ReceiveEvent(SKSECameraEvent* a_evn, EventDispatcher<SKSECameraEvent>*)
        -> EventResult
    {
        if (a_evn)
        {
            auto pc = PlayerCamera::GetSingleton();
            if (pc)
            {
                if ((a_evn->oldState && a_evn->oldState == pc->cameraStates[PlayerCamera::kCameraState_FirstPerson]) ||
                    (a_evn->newState && a_evn->newState == pc->cameraStates[PlayerCamera::kCameraState_FirstPerson]))
                {
                    QueueProcessWeaponDrawnChange(*g_thePlayer, DrawnState::Determine);
                }
            }
        }

        return kEvent_Continue;
    }

    void Controller::Receive(const Events::CreateWeaponNodesEvent& a_evn)
    {
        // we put the weapon on sheath node here regardless of ActorState since it should get redrawn anyway

        if (!a_evn.object) {
            return;
        }

        if (a_evn.object->formType != TESObjectWEAP::kTypeID) {
            return;
        }

        QueueActorTask(a_evn.reference, [this, left = a_evn.left, object = a_evn.object](Actor* a_actor)
        {
            auto pm = a_actor->processManager;
            if (!pm) {
                return;
            }

            auto form = left ?
                pm->equippedObject[ActorProcessManager::kEquippedHand_Left] :
                pm->equippedObject[ActorProcessManager::kEquippedHand_Right];

            if (form == object)
            {
                NiRootNodes roots(a_actor);
                roots.GetNPCRoots(m_strings->m_npcroot);

                ProcessEquippedWeapon(a_actor, roots, static_cast<TESObjectWEAP*>(form), false, left);
            }
        }
        );
    }

    void Controller::Receive(const Events::CreateArmorNodeEvent& a_evn)
    {
        if (!IsREFRValid(a_evn.reference)) {
            return;
        }

        if (a_evn.reference->formType != Actor::kTypeID) {
            return;
        }

        auto actor = static_cast<Actor*>(a_evn.reference.get());

        if ((m_conf.m_shield.m_flags & Flags::kNoImmediate) != Flags::kNoImmediate) {
            DoProcessEquippedShield(actor, DrawnState::Determine);
            //SetShieldGeometryCull(actor, DrawnState::Determine);
        }

        // and again just in case (deferred)
        QueueProcessEquippedShield(actor, DrawnState::Determine);

    }

    void Controller::Receive(const Events::OnSetEquipSlot& a_evn)
    {
        auto player = *g_thePlayer;
        if (!player || !player->loadedState) {
            return;
        }

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

                using size_type = decltype(pl->highActorHandles.count);

                for (size_type i = 0; i < pl->highActorHandles.count; i++)
                {
                    NiPointer<TESObjectREFR> ref;

                    if (!pl->highActorHandles[i].LookupREFR(ref)) {
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

#ifdef _SDS_UNUSED

    void ApplyNodeOverride(
        NiNode* a_root,
        const BSFixedString& a_node,
        const BSFixedString& a_parent)
    {
        if (NiPointer node = a_root->GetObjectByName(&a_node.data); node)
        {
            if (NiPointer newParentNode = FindNode(a_root, a_parent); newParentNode)
            {
                AttachToNode(node, newParentNode);
            }
        }
    }

    void Controller::ApplyNodeOverrides(
        const NiRootNodes& a_roots) const
    {
        for (auto& root : a_roots.m_nodes)
        {
            if (!root) {
                continue;
            }

            ApplyNodeOverride(root, m_strings->m_weaponSword, m_strings->m_weaponSwordOnBackMOV);
            ApplyNodeOverride(root, m_strings->m_weaponSwordLeft, m_strings->m_weaponSwordLeftOnBackMOV);

            ApplyNodeOverride(root, m_strings->m_weaponAxe, m_strings->m_weaponAxeOnBackMOV);
            ApplyNodeOverride(root, m_strings->m_weaponAxeLeft, m_strings->m_weaponAxeLeftOnBackMOV);

            ApplyNodeOverride(root, m_strings->m_weaponDagger, m_strings->m_weaponDaggerAnkleMOV);
            ApplyNodeOverride(root, m_strings->m_weaponDaggerLeft, m_strings->m_weaponDaggerLeftAnkleMOV);
        }
    }

#endif


}


