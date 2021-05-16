#include "pch.h"

#include "Controller.h"

#include "Util/Common.h"
#include "Util/Task.h"
#include "Util/Node.h"

namespace SDS
{
    using namespace Util::Common;
    using namespace Util::Task;
    using namespace Util::Node;
    using namespace Data;

    Controller::Controller(const Config& a_conf) :
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
        default: // Determine
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

        char buf[MAX_PATH];
        a_weapon->GetNodeName(buf);

        BSFixedString weaponNodeName(buf);

        for (std::size_t i = 0; i < std::size(a_roots.m_arr); i++)
        {
            auto root = a_roots.m_arr[i];

            if (!root) {
                continue;
            }

            if (i == 1 && (entry->m_flags & Data::Flags::kNoFirstPerson) == Data::Flags::kNoFirstPerson) {
                continue;
            }

            NiPointer<NiNode> sheathedNode, drawnNode;
            if (!GetParentNodes(entry, root, a_left, sheathedNode, drawnNode)) {
                return;
            }

            auto& sourceNode = a_drawn ? sheathedNode : drawnNode;
            auto& targetNode = a_drawn ? drawnNode : sheathedNode;

            NiPointer<NiAVObject> weaponNode;
            GetNiObject(weaponNodeName, sourceNode, weaponNode);

            if (weaponNode)
            {
                weaponNode->m_flags &= ~NiAVObject::kFlag_Cull;

                /*gLog.Debug("[%.8X] [%s] [WEAP: %X | left: %d] Attaching [%s] from [%s] to [%s] [root: %s]",
                    a_actor->formID.get(),
                    a_actor->GetReferenceName(),
                    a_weapon->formID.get(),
                    a_left,
                    weaponNode->m_name,
                    sourceNode->m_name,
                    targetNode->m_name,
                    root->m_name);*/


                AttachToNode(weaponNode, targetNode);

            }
            else
            {
                // if the weapon node is attached to target and invisible just remove the cull flag

                GetNiObject(weaponNodeName, targetNode, weaponNode);
                if (weaponNode)
                {
                    if ((weaponNode->m_flags & NiAVObject::kFlag_Cull) == NiAVObject::kFlag_Cull)
                    {
                        /*gLog.Debug("[%.8X] [%s] [WEAP: %X | left: %d] Clearing cull flag [%s] [root: %s]",
                            a_actor->formID.get(),
                            a_actor->GetReferenceName(),
                            a_weapon->formID.get(),
                            a_left,
                            weaponNode->m_name,
                            root->m_name);*/

                        weaponNode->m_flags &= ~NiAVObject::kFlag_Cull;
                    }
                }
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

        auto form = pm->equippedObject[ActorProcessManager::kEquippedHand_Left];
        if (form)
        {
            if (form->formType == TESObjectWEAP::kTypeID)
            {
                ProcessEquippedWeapon(a_actor, roots, static_cast<TESObjectWEAP*>(form), a_drawn, true);
            }
        }

        form = pm->equippedObject[ActorProcessManager::kEquippedHand_Right];
        if (form)
        {
            if (form->formType == TESObjectWEAP::kTypeID)
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

    NiNode* Controller::GetScbAttachmentNode(
        TESObjectREFR* a_actor,
        TESForm* a_form,
        NiAVObject* a_sheatheNode,
        bool a_checkEquippedLeft)
    {
        if (!a_actor || !a_form) {
            return nullptr;
        }

        if (a_actor->formType != Actor::kTypeID) {
            return nullptr;
        }

        if (a_form->formType != TESObjectWEAP::kTypeID) {
            return nullptr;
        }

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

        bool no1p = (entry->m_flags & Data::Flags::kNoFirstPerson) == Data::Flags::kNoFirstPerson;

        NiRootNodes roots(a_actor, no1p);

        auto root = a_sheatheNode->GetAsNiNode();
        if (!root) {
            root = a_sheatheNode->m_parent;
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

        return entry->GetNode(found, true);
    }


    auto Controller::ReceiveEvent(TESObjectLoadedEvent* a_evn, EventDispatcher<TESObjectLoadedEvent>*)
        -> EventResult
    {
        if (a_evn && a_evn->loaded)
        {
            auto form = a_evn->formId.Lookup();
            if (form && form->formType == Actor::kTypeID) {
                QueueProcessWeaponDrawnChange(static_cast<TESObjectREFR*>(form), DrawnState::Determine);
            }
        }

        return kEvent_Continue;
    }

    auto Controller::ReceiveEvent(TESInitScriptEvent* a_evn, EventDispatcher<TESInitScriptEvent>*)
        -> EventResult
    {
        if (a_evn) {
            QueueProcessWeaponDrawnChange(a_evn->reference, DrawnState::Determine);
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

    // prob pointless

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
        // we put the weapon on sheathe node here regardless of ActorState since it should get redrawn anyway

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
                ProcessEquippedWeapon(a_actor, a_actor, static_cast<TESObjectWEAP*>(form), false, left);
            }
        }
        );

    }

}