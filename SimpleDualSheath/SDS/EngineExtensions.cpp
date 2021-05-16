#include "pch.h"

#include "EngineExtensions.h"

#include <ext/JITASM.h>

namespace SDS
{
    std::unique_ptr<EngineExtensions> EngineExtensions::m_Instance;

    EngineExtensions::EngineExtensions(const std::shared_ptr<Controller>& a_controller)
    {
        Patch_CreateWeaponNodes();

        if (a_controller->GetConfig().m_scb) 
        {
            Patch_SCB_Attach();
            Patch_SCB_Detach();
        }

        m_dispatchers.m_createWeaponNodes.AddSink(a_controller.get());

        m_controller = a_controller;
    }

    void EngineExtensions::Initialize(const std::shared_ptr<Controller>& a_controller)
    {
        ASSERT(m_Instance.get() == nullptr);

        m_Instance = std::unique_ptr<EngineExtensions>{ new EngineExtensions(a_controller) };
    }

    void EngineExtensions::Patch_CreateWeaponNodes()
    {
        struct Assembly : JITASM::JITASM {
            Assembly(std::uintptr_t a_targetAddr
            ) : JITASM(ISKSE::GetLocalTrampoline())
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

    void EngineExtensions::Patch_SCB_Attach()
    {
        struct Assembly : JITASM::JITASM {
            Assembly(std::uintptr_t targetAddr
            ) : JITASM(ISKSE::GetLocalTrampoline())
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
                mov(rdx, ptr[rcx + rdx + 0x10]); // object
                mov(rcx, ptr[rbp - 0x31]); // actor
                mov(r8, rsi); // object attachment node
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
                mov(rsi, rdx); // should be safe to clobber rsi here

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
        struct Assembly : JITASM::JITASM {
            Assembly(std::uintptr_t targetAddr
            ) : JITASM(ISKSE::GetLocalTrampoline())
            {
                Xbyak::Label callLabel;
                Xbyak::Label retnLabel;
                Xbyak::Label exitContinue;
                Xbyak::Label exitIsShield;

                Xbyak::Label isWeaponSlot;
                Xbyak::Label ok;

                jne(isWeaponSlot);

                mov(rcx, ptr[rbp - 0x20]); // reference
                mov(rdx, ptr[rdi]); // object (TESForm)
                mov(r8, r12); // object attachment node
                xor_(r9d, r9d);
                call(ptr[rip + callLabel]);
                test(rax, rax);
                jne(ok);
                jmp(ptr[rip + exitIsShield]);

                L(ok);
                mov(r12, rax);

                L(isWeaponSlot);
                jmp(ptr[rip + exitContinue]);

                L(exitIsShield);
                dq(targetAddr + 0xB9);

                L(exitContinue);
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

    NiNode* EngineExtensions::GetScbAttachmentNode(
        TESObjectREFR* a_actor,
        TESForm* a_form,
        NiAVObject* a_sheatheNode,
        bool a_checkEquippedLeft)
    {
        return m_Instance->m_controller->GetScbAttachmentNode(a_actor, a_form, a_sheatheNode, a_checkEquippedLeft);
    }
}