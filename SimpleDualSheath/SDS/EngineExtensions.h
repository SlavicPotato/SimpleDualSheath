#pragma once

#include "Controller.h"

#include "Events/Dispatcher.h"
#include "Events/CreateWeaponNodesEvent.h"

namespace SDS
{
    class EngineExtensions
    {
    public:

        static void Initialize(const std::shared_ptr<Controller>& a_controller);

    private:
        EngineExtensions(const std::shared_ptr<Controller>& a_controller);

        void Patch_CreateWeaponNodes();
        void Patch_SCB_Attach();
        void Patch_SCB_Detach();

        static void CreateWeaponNodes_Hook(TESObjectREFR* a_actor, TESForm* a_object, bool a_left);
        decltype(CreateWeaponNodes_Hook)* m_createWeaponNodes_o;

        static NiNode* GetScbAttachmentNode(TESObjectREFR* a_actor, TESForm* a_form, NiAVObject* a_sheatheNode, bool a_checkEquippedLeft);

        struct
        {
            Events::EventDispatcher<Events::CreateWeaponNodesEvent> m_createWeaponNodes;
        } m_dispatchers;

        std::shared_ptr<Controller> m_controller;

        inline static auto m_createWeaponNodes_a = IAL::Addr<std::uintptr_t>(19342);
        inline static auto m_scbAttach_a = IAL::Addr(15569, 0x3A3);
        inline static auto m_scbDetach_a = IAL::Addr(15496, 0x1A3);

        static std::unique_ptr<EngineExtensions> m_Instance;
    };
}