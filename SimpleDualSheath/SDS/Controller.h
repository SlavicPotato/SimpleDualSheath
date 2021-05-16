#pragma once

#include "StringHolder.h"
#include "Data.h"
#include "Util/Node.h"
#include "Config.h"

#include "Events/Dispatcher.h"
#include "Events/CreateWeaponNodesEvent.h"

namespace SDS
{
    class Controller :
        public BSTEventSink <TESObjectLoadedEvent>,
        public BSTEventSink <TESInitScriptEvent>,
        public BSTEventSink <SKSENiNodeUpdateEvent>,
        public BSTEventSink <SKSEActionEvent>,
        public BSTEventSink <SKSECameraEvent>,
        public Events::EventSink<Events::CreateWeaponNodesEvent>
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
        NiNode* GetScbAttachmentNode(TESObjectREFR* a_actor, TESForm* a_form, NiAVObject* a_sheatheNode, bool a_checkEquippedLeft);

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetConfig() {
            return m_conf;
        }

    private:

        [[nodiscard]] bool GetParentNodes(const Data::Weapon* a_entry, NiNode* a_root, bool a_left, NiPointer<NiNode>& a_sheathedNode, NiPointer<NiNode>& a_drawnNode) const;
        void ProcessEquippedWeapon(Actor* a_actor, const Util::Node::NiRootNodes& a_roots, TESObjectWEAP* a_weapon, bool a_drawn, bool a_left) const;
        void ProcessWeaponDrawnChange(Actor* a_actor, bool a_drawn) const;
        void QueueProcessWeaponDrawnChange(TESObjectREFR* a_actor, DrawnState a_drawnState) const;

        static bool GetIsDrawn(Actor* a_actor, DrawnState a_state);

        virtual EventResult	ReceiveEvent(TESObjectLoadedEvent* a_evn, EventDispatcher<TESObjectLoadedEvent>* a_dispatcher) override;
        virtual EventResult	ReceiveEvent(TESInitScriptEvent* a_evn, EventDispatcher<TESInitScriptEvent>* a_dispatcher) override;
        virtual EventResult	ReceiveEvent(SKSENiNodeUpdateEvent* a_evn, EventDispatcher<SKSENiNodeUpdateEvent>* a_dispatcher) override;
        virtual EventResult	ReceiveEvent(SKSEActionEvent* a_evn, EventDispatcher<SKSEActionEvent>* a_dispatcher) override;
        virtual EventResult	ReceiveEvent(SKSECameraEvent* a_evn, EventDispatcher<SKSECameraEvent>* a_dispatcher) override;

        virtual void Receive(const Events::CreateWeaponNodesEvent& a_evn) override;

        Config m_conf;
        std::unique_ptr<StringHolder> m_strings;
        std::unique_ptr<Data::WeaponData> m_data;
    };
}