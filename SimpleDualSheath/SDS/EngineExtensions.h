#pragma once

#include "Controller.h"

#include "Events/Dispatcher.h"
#include "Events/CreateWeaponNodesEvent.h"
#include "Events/CreateArmorNodeEvent.h"
#include "Events/OnSetEquipSlot.h"

namespace SDS
{
    class BShkbAnimationGraph;

    class EngineExtensions : ILog
    {
    public:

        EngineExtensions(const EngineExtensions&) = delete;
        EngineExtensions(EngineExtensions&&) = delete;
        EngineExtensions& operator=(const EngineExtensions&) = delete;
        EngineExtensions& operator=(EngineExtensions&&) = delete;

        enum class MemoryValidationFlags : std::uint8_t
        {
            kNone = 0,

            kCreateWeaponNodes = 1ui8 << 0,
            kCreateArmorNode = 1ui8 << 1,
            kDisableShieldHideOnSit = 1ui8 << 2,
            kScabbardAttach = 1ui8 << 3,
            kScabbardDetach = 1ui8 << 4
        };

        static void Initialize(const std::shared_ptr<Controller>& a_controller);
        static MemoryValidationFlags ValidateMemory(const Config& a_config);

        SKMP_FORCEINLINE static auto GetSingleton() {
            return m_Instance.get();
        }
        
        SKMP_FORCEINLINE auto &GetEventDispatchers() {
            return m_dispatchers;
        }

        FN_NAMEPROC("EngineExtensions");

    private:
        EngineExtensions(const std::shared_ptr<Controller>& a_controller);

        void Patch_CreateWeaponNodes();
        void Patch_CreateArmorNode();
        void Patch_SCB_Attach();
        void Patch_SCB_Detach();
        void Patch_DisableShieldHideOnSit(const Config& a_config);
        bool Hook_TESObjectWEAP_SetEquipSlot();
        bool Patch_ShieldHandWorkaround();

        static void CreateWeaponNodes_Hook(TESObjectREFR* a_actor, TESForm* a_object, bool a_left);
        static NiAVObject* CreateArmorNode_Hook(NiAVObject* a_obj, Biped* a_info, BipedParam* a_params);
        static NiNode* GetScbAttachmentNode_Hook(TESObjectREFR* a_actor, TESForm* a_form, NiAVObject* a_sheatheNode, bool a_checkEquippedLeft);
        static void TESObjectWEAP_SetEquipSlot_Hook(BGSEquipType *a_this, BGSEquipSlot* a_slot);

        static bool Unk140609D50_BShkbAnimationGraph_SetGraphVariableInt_Hook(BShkbAnimationGraph* a_graph, const BSFixedString& a_name, std::int32_t a_value, Actor* a_actor);
        static std::uint32_t Unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_Hook(IAnimationGraphManagerHolder* a_holder, const BSFixedString& a_name, std::int32_t a_value, Actor* a_actor);
        static std::uint32_t Unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_Hook(IAnimationGraphManagerHolder* a_holder, const BSFixedString& a_name, std::int32_t a_value, Actor* a_actor);

        typedef bool(*BShkbAnimationGraph_SetGraphVariableInt_t)(BShkbAnimationGraph* a_graph, const BSFixedString& a_name, std::int32_t a_value);
        typedef std::uint32_t(*IAnimationGraphManagerHolder_SetVariableOnGraphsInt_t)(IAnimationGraphManagerHolder* a_holder, const BSFixedString& a_name, std::int32_t a_value);

        static bool ShouldBlockShieldHide(Actor *a_actor);

        decltype(&CreateWeaponNodes_Hook) m_createWeaponNodes_o;
        decltype(&TESObjectWEAP_SetEquipSlot_Hook) m_TESObjectWEAP_SetEquipSlot_o;
        BShkbAnimationGraph_SetGraphVariableInt_t m_BShkbAnimationGraph_SetGraphVariableInt_o;
        IAnimationGraphManagerHolder_SetVariableOnGraphsInt_t m_unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_o;
        IAnimationGraphManagerHolder_SetVariableOnGraphsInt_t m_unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_o;

        struct
        {
            Events::EventDispatcher<Events::CreateWeaponNodesEvent> m_createWeaponNodes;
            Events::EventDispatcher<Events::CreateArmorNodeEvent> m_createArmorNode;
            Events::EventDispatcher<Events::OnSetEquipSlot> m_setEquipSlot;
        } m_dispatchers;

        std::shared_ptr<Controller> m_controller;

        inline static auto m_createWeaponNodes_a = IAL::Address<std::uintptr_t>(19342);
        inline static auto m_createArmorNode_a = IAL::Address<std::uintptr_t>(15501, 0xB58);
        inline static auto m_scbAttach_a = IAL::Address<std::uintptr_t>(15569, 0x3A3);
        inline static auto m_scbDetach_a = IAL::Address<std::uintptr_t>(15496, 0x1A3);
        inline static auto m_hideShield_a = IAL::Address<std::uintptr_t>(36580, 0x6); // does other stuff but we don't care here
        inline static auto m_vtbl_TESObjectWEAP = IAL::Address<std::uintptr_t>(234396);

        inline static auto m_unk140609D50_BShkbAnimationGraph_SetGraphVariableInt_a = IAL::Address<std::uintptr_t>(36957, 0x1DA);               // load (iLeftHandType), rbp - 38 = Actor (BShkbAnimationGraph::SetGraphVariableInt)
        inline static auto m_unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_a = IAL::Address<std::uintptr_t>(36949, 0x48);    // equip (iLeftHandType), rsi = Actor (IAnimationGraphManagerHolder::SetVariableOnGraphsInt)
        inline static auto m_unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_a = IAL::Address<std::uintptr_t>(37866, 0x17B);   // draw (iLeftHandEquipped), r15 - B8 = Actor (IAnimationGraphManagerHolder::SetVariableOnGraphsInt)

        static std::unique_ptr<EngineExtensions> m_Instance;
    };

    DEFINE_ENUM_CLASS_BITWISE(EngineExtensions::MemoryValidationFlags);
}