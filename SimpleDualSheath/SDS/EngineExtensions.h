#pragma once

#include "Controller.h"

#include "Events/Dispatcher.h"
#include "Events/CreateWeaponNodesEvent.h"
#include "Events/CreateArmorNodeEvent.h"
#include "Events/OnSetEquipSlot.h"

namespace SDS
{
    class EngineExtensions : ILog
    {
    public:

        static void Initialize(const std::shared_ptr<Controller>& a_controller);
        static bool ValidateMemory(const Controller* a_controller);

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
        bool Hook_TESObjectWEAP_SetEquipSlot();

        static void CreateWeaponNodes_Hook(TESObjectREFR* a_actor, TESForm* a_object, bool a_left);
        decltype(CreateWeaponNodes_Hook)* m_createWeaponNodes_o;
        static NiAVObject* CreateArmorNode_Hook(NiAVObject* a_obj, Biped* a_info, BipedParam* a_params);

        static NiNode* GetScbAttachmentNode_Hook(TESObjectREFR* a_actor, TESForm* a_form, NiAVObject* a_sheatheNode, bool a_checkEquippedLeft);

        static void TESObjectWEAP_SetEquipSlot_Hook(BGSEquipType *a_this, BGSEquipSlot* a_slot);
        decltype(TESObjectWEAP_SetEquipSlot_Hook)* m_TESObjectWEAP_SetEquipSlot_o;

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
        inline static auto m_vtbl_TESObjectWEAP = IAL::Address<std::uintptr_t>(234396);

        static std::unique_ptr<EngineExtensions> m_Instance;
    };
}