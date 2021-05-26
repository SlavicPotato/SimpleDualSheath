#pragma once

#include "StringHolder.h"
#include "Data.h"
#include "Util/Node.h"
#include "Config.h"
#include "EquipManager.h"

#include "Events/Dispatcher.h"
#include "Events/CreateWeaponNodesEvent.h"
#include "Events/CreateArmorNodeEvent.h"
#include "Events/OnSetEquipSlot.h"

namespace SDS
{
    class Controller :
        public EquipExtensions,
        public BSTEventSink <TESObjectLoadedEvent>,
        public BSTEventSink <TESInitScriptEvent>,
        public BSTEventSink <TESEquipEvent>,
        public BSTEventSink <SKSENiNodeUpdateEvent>,
        public BSTEventSink <SKSEActionEvent>,
        public BSTEventSink <SKSECameraEvent>,
        public Events::EventSink<Events::CreateWeaponNodesEvent>,
        public Events::EventSink<Events::CreateArmorNodeEvent>,
        public Events::EventSink<Events::OnSetEquipSlot>
    {

        enum class DrawnState : std::uint8_t
        {
            Determine,
            Sheathed,
            Drawn
        };

    public:
        Controller(const Config& a_conf);

        void InitializeData();
        [[nodiscard]] NiNode* GetScbAttachmentNode(TESObjectREFR* a_actor, TESForm* a_form, NiAVObject* a_sheatheNode, bool a_checkEquippedLeft) const;

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetConfig() const {
            return m_conf;
        }

    private:

        [[nodiscard]] bool GetParentNodes(const Data::Weapon* a_entry, NiNode* a_root, bool a_left, NiPointer<NiNode>& a_sheathedNode, NiPointer<NiNode>& a_drawnNode) const;

        void ProcessEquippedWeapon(Actor* a_actor, const Util::Node::NiRootNodes& a_roots, TESObjectWEAP* a_weapon, bool a_drawn, bool a_left) const;
        void ProcessWeaponDrawnChange(Actor* a_actor, bool a_drawn) const;
        void QueueProcessWeaponDrawnChange(TESObjectREFR* a_actor, DrawnState a_drawnState) const;

        [[nodiscard]] bool IsShieldAllowed(Actor* a_actor) const;
        void ProcessEquippedShield(Actor* a_actor, const Util::Node::NiRootNodes& a_roots, TESObjectARMO* a_armor, bool a_drawn) const;
        void DoProcessEquippedShield( Actor* a_actor, DrawnState a_drawnState) const;
        void QueueProcessEquippedShield(Actor* a_actor, DrawnState a_drawnState) const;

        [[nodiscard]] static bool GetIsDrawn(Actor* a_actor, DrawnState a_state);

        void OnActorLoad(TESObjectREFR* a_actor);
        void OnShieldEquip(Actor *a_actor, TESObjectARMO *a_armor);
        void OnWeaponEquip(Actor *a_actor, TESObjectWEAP *a_weapon);

        // Beth
        virtual EventResult	ReceiveEvent(TESObjectLoadedEvent* a_evn, EventDispatcher<TESObjectLoadedEvent>* a_dispatcher) override;
        virtual EventResult	ReceiveEvent(TESInitScriptEvent* a_evn, EventDispatcher<TESInitScriptEvent>* a_dispatcher) override;
        virtual EventResult	ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>* dispatcher) override; // shield only

        // SKSE
        virtual EventResult	ReceiveEvent(SKSENiNodeUpdateEvent* a_evn, EventDispatcher<SKSENiNodeUpdateEvent>* a_dispatcher) override;
        virtual EventResult	ReceiveEvent(SKSEActionEvent* a_evn, EventDispatcher<SKSEActionEvent>* a_dispatcher) override;
        virtual EventResult	ReceiveEvent(SKSECameraEvent* a_evn, EventDispatcher<SKSECameraEvent>* a_dispatcher) override;

        // EngineExtensions
        virtual void Receive(const Events::CreateWeaponNodesEvent& a_evn) override;
        virtual void Receive(const Events::CreateArmorNodeEvent& a_evn) override; // shield only
        virtual void Receive(const Events::OnSetEquipSlot& a_evn) override; 

        Config m_conf;

        std::unique_ptr<StringHolder> m_strings;
        std::unique_ptr<Data::WeaponData> m_data;

#ifdef _SDS_UNUSED
        void Controller::ApplyNodeOverrides(const Util::Node::NiRootNodes& a_roots) const;
#endif
    };
}