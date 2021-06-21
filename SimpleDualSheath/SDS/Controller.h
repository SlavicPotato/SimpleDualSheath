#pragma once

#include "StringHolder.h"
#include "Data.h"
#include "Util/Node.h"
#include "Config.h"
#include "EquipManager.h"

#ifdef _SDS_UNUSED
#include "NodeOverride.h"
#endif

#include "Events/Dispatcher.h"
#include "Events/CreateWeaponNodesEvent.h"
#include "Events/CreateArmorNodeEvent.h"
#include "Events/OnSetEquipSlot.h"

#include <ext/Threads.h>

namespace SDS
{
    class Controller :
        public EquipExtensions,
        public BSTEventSink <TESObjectLoadedEvent>,
        public BSTEventSink <TESInitScriptEvent>,
        public BSTEventSink <TESEquipEvent>,
#ifdef _SDS_UNUSED
        public BSTEventSink <SKSENiNodeUpdateEvent>,
#endif
        public BSTEventSink <SKSEActionEvent>,
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

        Controller(const Controller&) = delete;
        Controller(Controller&&) = delete;
        Controller& operator=(const Controller&) = delete;
        Controller& operator=(Controller&&) = delete;

        void InitializeData();
        [[nodiscard]] NiNode* GetScbAttachmentNode(Actor* a_actor, TESForm* a_form, NiNode* a_attachmentNode) const;
        [[nodiscard]] const BSFixedString* GetWeaponAttachmentNodeName(Actor* a_actor, TESForm* a_form, bool a_firstPerson, bool a_left) const;
        [[nodiscard]] const BSFixedString* GetShieldAttachmentNodeName(Actor* a_actor, TESForm* a_form, bool a_firstPerson) const;
        

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetConfig() const {
            return m_conf;
        }
        
        [[nodiscard]] SKMP_FORCEINLINE const auto GetStringHolder() const {
            return m_strings.get();
        }

        [[nodiscard]] bool IsShieldEnabled(Actor* a_actor) const;
        [[nodiscard]] bool ShouldBlockShieldHide(Actor* a_actor) const;
        [[nodiscard]] static std::uint32_t GetShieldBipedObject(Actor* a_actor);

        [[nodiscard]] void EvaluateDrawnStateOnNearbyActors();

    private:

        [[nodiscard]] bool GetParentNodes(const Data::Weapon* a_entry, NiNode* a_root, bool a_left, NiPointer<NiNode>& a_sheathedNode, NiPointer<NiNode>& a_drawnNode) const;

        void ProcessEquippedWeapon(Actor* a_actor, const Util::Node::NiRootNodes& a_roots, TESObjectWEAP* a_weapon, bool a_drawn, bool a_left) const;
        void ProcessWeaponDrawnChange(Actor* a_actor, bool a_drawn) const;
        void QueueProcessWeaponDrawnChange(TESObjectREFR* a_actor, DrawnState a_drawnState) const;

        void ProcessEquippedShield(Actor* a_actor, const Util::Node::NiRootNodes& a_roots, bool a_drawn) const;

        [[nodiscard]] NiNode* FindObjectNPCRoot(TESObjectREFR* a_actor, NiAVObject* a_object, bool a_no1p) const;

        [[nodiscard]] static bool GetIsDrawn(Actor* a_actor, DrawnState a_state);

        void OnActorLoad(TESObjectREFR* a_actor);
#ifdef _SDS_UNUSED
        void OnNiNodeUpdate(TESObjectREFR* a_actor);
#endif
        void OnWeaponEquip(Actor* a_actor, TESObjectWEAP* a_weapon);

        // Beth
        virtual EventResult	ReceiveEvent(TESObjectLoadedEvent* a_evn, EventDispatcher<TESObjectLoadedEvent>* a_dispatcher) override;
        virtual EventResult	ReceiveEvent(TESInitScriptEvent* a_evn, EventDispatcher<TESInitScriptEvent>* a_dispatcher) override;
        virtual EventResult	ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>* dispatcher) override; // shield only

        // SKSE
#ifdef _SDS_UNUSED
        virtual EventResult	ReceiveEvent(SKSENiNodeUpdateEvent* a_evn, EventDispatcher<SKSENiNodeUpdateEvent>* a_dispatcher) override;
#endif
        virtual EventResult	ReceiveEvent(SKSEActionEvent* a_evn, EventDispatcher<SKSEActionEvent>* a_dispatcher) override;
        //virtual EventResult	ReceiveEvent(SKSECameraEvent* a_evn, EventDispatcher<SKSECameraEvent>* a_dispatcher) override;

        // EngineExtensions
        virtual void Receive(const Events::OnSetEquipSlot& a_evn) override;

        Config m_conf;

        std::shared_ptr<StringHolder> m_strings;
        std::unique_ptr<Data::WeaponData> m_data;

        //mutable WCriticalSection m_lock;

#ifdef _SDS_UNUSED
        std::unique_ptr<NodeOverride> m_nodeOverride;
#endif

    };
}