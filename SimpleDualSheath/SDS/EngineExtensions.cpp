#include "pch.h"

#include "EngineExtensions.h"
#include "SDS/Data.h"

#include <ext/JITASM.h>
#include <ext/Patching.h>

namespace SDS
{
    using namespace JITASM;

    std::unique_ptr<EngineExtensions> EngineExtensions::m_Instance;

    EngineExtensions::EngineExtensions(
        const std::shared_ptr<Controller>& a_controller)
    {
        Patch_CreateWeaponNodes();

        auto& config = a_controller->GetConfig();

        if ((config.m_shield & Data::Flags::kEnabled) != Data::Flags::kNone)
        {
            Patch_CreateArmorNode();
            m_dispatchers.m_createArmorNode.AddSink(a_controller.get());
        }

        if (config.m_scb)
        {
            Patch_SCB_Attach();
            Patch_SCB_Detach();
        }

        m_dispatchers.m_createWeaponNodes.AddSink(a_controller.get());

        m_controller = a_controller;
    }

    void EngineExtensions::Initialize(
        const std::shared_ptr<Controller>& a_controller)
    {
        if (m_Instance.get() == nullptr) {
            m_Instance = std::unique_ptr<EngineExtensions>{ new EngineExtensions(a_controller) };
        }
    }

    bool EngineExtensions::ValidateMemory(
        const Controller* a_controller)
    {
        using namespace Patching;

        constexpr std::uint8_t d_createWeaponNodes[]{ 0x40, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56 };
        if (!validate_mem(m_createWeaponNodes_a, d_createWeaponNodes)) {
            return false;
        }

        auto& config = a_controller->GetConfig();

        if ((config.m_shield & Data::Flags::kEnabled) != Data::Flags::kNone)
        {
            constexpr std::uint8_t d_createArmorNodev1[]{ 0x48, 0x85, 0xC0, 0x74, 0x0D };
            constexpr std::uint8_t d_createArmorNodev2[]{ 0xE9 };

            if (!validate_mem(m_createArmorNode_a, d_createArmorNodev1) && 
                !validate_mem(m_createArmorNode_a, d_createArmorNodev2)) 
            {
                return false;
            }
        }

        if (config.m_scb)
        {
            constexpr std::uint8_t d_scbAttach[]{ 0x80, 0x7D, 0x6F, 0x00, 0x75, 0x1E };
            if (!validate_mem(m_scbAttach_a, d_scbAttach)) {
                return false;
            }

            constexpr std::uint8_t d_scbDetach[]{ 0x0F, 0x84, 0xB3, 0x00, 0x00, 0x00 };
            if (!validate_mem(m_scbDetach_a, d_scbDetach)) {
                return false;
            }
        }

        return true;
    }

    void EngineExtensions::Patch_CreateWeaponNodes()
    {
        struct Assembly : JITASM
        {
            Assembly(std::uintptr_t a_targetAddr) :
                JITASM(ISKSE::GetLocalTrampoline())
            {
                Xbyak::Label retnLabel;

                db(reinterpret_cast<Xbyak::uint8*>(a_targetAddr), 0x7);
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(a_targetAddr + 0x7);
            }
        };

        Assembly code(m_createWeaponNodes_a);
        m_createWeaponNodes_o = code.get<decltype(m_createWeaponNodes_o)>();

        ISKSE::GetBranchTrampoline().Write6Branch(m_createWeaponNodes_a, std::uintptr_t(CreateWeaponNodes_Hook));
    }

    void EngineExtensions::Patch_CreateArmorNode()
    {
        struct Assembly : JITASM {
            Assembly(std::uintptr_t a_addr, bool a_chain) :
                JITASM(ISKSE::GetLocalTrampoline())
            {
                Xbyak::Label callLabel;
                Xbyak::Label nullLabel;
                Xbyak::Label retnNullLabel;
                Xbyak::Label retnOKLabel;

                mov(rcx, rax);
                mov(rdx, r13);
                mov(r8, ptr[rsp + 0x78]);
                call(ptr[rip + callLabel]);

                if (a_chain)
                {
                    jmp(ptr[rip + retnOKLabel]);
                }
                else
                {
                    test(rax, rax);
                    je(nullLabel);
                    jmp(ptr[rip + retnOKLabel]);
                    L(nullLabel);
                    jmp(ptr[rip + retnNullLabel]);
                }

                L(retnOKLabel);
                if (a_chain)
                {
                    dq(a_addr);
                }
                else
                {
                    dq(a_addr + 0x5);

                    L(retnNullLabel);
                    dq(a_addr + 0x12);
                }

                L(callLabel);
                dq(std::uintptr_t(CreateArmorNode_Hook));
            }
        };

        std::uintptr_t jmpAddr;
        bool chain = ::Hook::GetDst5<0xE9>(m_createArmorNode_a, jmpAddr); // if cbp already hooked here
        if (!chain) {
            jmpAddr = m_createArmorNode_a;
        }

        Assembly code(jmpAddr, chain);
        ISKSE::GetBranchTrampoline().Write5Branch(m_createArmorNode_a, code.get());

    }

    void EngineExtensions::Patch_SCB_Attach()
    {
        struct Assembly : JITASM
        {
            Assembly(std::uintptr_t targetAddr) :
                JITASM(ISKSE::GetLocalTrampoline())
            {
                Xbyak::Label callLabel;
                Xbyak::Label exitContinue;
                Xbyak::Label exitSkip;

                Xbyak::Label cont;
                Xbyak::Label skip;

                cmp(byte[rbp + 0x6F], 0);
                je(cont);

                mov(rcx, ptr[rbp + 0x77]);
                mov(rcx, ptr[rcx]);
                imul(rdx, r14, 0x78);
                mov(rdx, ptr[rcx + rdx + 0x10]); // form
                mov(rcx, ptr[rbp - 0x31]); // reference
                mov(r8, rsi); // attachment node
                mov(r9b, 0x1);

                push(rax);
                sub(rsp, 0x20);

                //mov(r9, rbx); // scb node

                call(ptr[rip + callLabel]);
                mov(rdx, rax);

                add(rsp, 0x20);
                pop(rax);

                test(rdx, rdx);
                je(skip);
                mov(rsi, rdx); // rsi: should be unused after scb is attached

                L(cont);
                jmp(ptr[rip + exitContinue]);

                L(skip);
                jmp(ptr[rip + exitSkip]);

                L(exitContinue);
                dq(targetAddr + 0x6);

                L(exitSkip);
                dq(targetAddr + 0x24);

                L(callLabel);
                dq(std::uintptr_t(GetScbAttachmentNode));
            }
        };

        Assembly code(m_scbAttach_a);
        ISKSE::GetBranchTrampoline().Write6Branch(m_scbAttach_a, code.get());
    }

    void EngineExtensions::Patch_SCB_Detach()
    {
        struct Assembly : JITASM
        {
            Assembly(std::uintptr_t targetAddr) :
                JITASM(ISKSE::GetLocalTrampoline())
            {
                Xbyak::Label callLabel;
                Xbyak::Label retnLabel;
                Xbyak::Label exitIsWeapon;
                Xbyak::Label exitIsShield;

                Xbyak::Label isWeaponSlot;
                Xbyak::Label ok;

                jne(isWeaponSlot);

                mov(rcx, ptr[rbp - 0x20]); // reference
                mov(rdx, ptr[rdi]); // form
                mov(r8, r12); // attachment node
                xor_(r9d, r9d);
                call(ptr[rip + callLabel]);
                test(rax, rax);
                jne(ok);
                jmp(ptr[rip + exitIsShield]);

                L(ok);
                mov(r12, rax);

                L(isWeaponSlot);
                jmp(ptr[rip + exitIsWeapon]);

                L(exitIsShield);
                dq(targetAddr + 0xB9);

                L(exitIsWeapon);
                dq(targetAddr + 0x6);

                L(callLabel);
                dq(std::uintptr_t(GetScbAttachmentNode));
            }
        };

        Assembly code(m_scbDetach_a);
        ISKSE::GetBranchTrampoline().Write6Branch(m_scbDetach_a, code.get());
    }

    void EngineExtensions::CreateWeaponNodes_Hook(
        TESObjectREFR* a_actor,
        TESForm* a_object,
        bool a_left)
    {
        m_Instance->m_createWeaponNodes_o(a_actor, a_object, a_left);

        Events::CreateWeaponNodesEvent evn{ a_actor, a_object,  a_left };
        m_Instance->m_dispatchers.m_createWeaponNodes.SendEvent(evn);
    }

    NiAVObject* EngineExtensions::CreateArmorNode_Hook(
        NiAVObject* a_obj,
        Biped* a_info,
        BipedParam* a_params)
    {
        if (a_info)
        {
            Events::CreateArmorNodeEvent evn;

            if (!a_info->handle.LookupREFR(evn.reference)) {
                evn.reference = nullptr;
            }
            evn.object = a_obj;
            evn.info = a_info;
            evn.params = a_params;

            m_Instance->m_dispatchers.m_createArmorNode.SendEvent(evn);
        }

        return a_obj;
    }

    NiNode* EngineExtensions::GetScbAttachmentNode(
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

        return m_Instance->m_controller->GetScbAttachmentNode(a_actor, a_form, a_sheatheNode, a_checkEquippedLeft);
    }
}