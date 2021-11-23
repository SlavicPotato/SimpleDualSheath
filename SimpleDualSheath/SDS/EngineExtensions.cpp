#include "pch.h"

#include "EngineExtensions.h"
#include "SDS/Data.h"
#include "SDS/Util/Common.h"
#include "SDS/Util/Node.h"

#include <ext/IHook.h>
#include <ext/JITASM.h>
#include <ext/Patching.h>
#include <ext/VFT.h>

namespace SDS
{
	using namespace JITASM;
	using namespace Events;
	using namespace Util;

	std::unique_ptr<EngineExtensions> EngineExtensions::m_Instance;

	EngineExtensions::EngineExtensions(
		const std::shared_ptr<Controller>& a_controller)
	{
		auto& config = a_controller->GetConfig();

		Patch_WeaponObjects_Attach();
		Patch_SCB_Get();

		if (config.m_scb)
		{
			Patch_SCB_Attach();
			Patch_SCB_Detach();
		}

		if (config.m_shield.IsEnabled())
		{
			Patch_ShieldObject_Attach();
			if (config.m_shieldHandWorkaround)
			{
				if (!Patch_ShieldHandWorkaround())
				{
					Error("Shield hand patches failed");
				}
			}
		}

		if (config.m_shieldHideFlags.test_any(Data::Flags::kEnabled))
		{
			Patch_DisableShieldHideOnSit(config);
		}

		if (config.HasEnabled2HEntries())
		{
			if (Hook_TESObjectWEAP_SetEquipSlot())
			{
				m_dispatchers.m_setEquipSlot.AddSink(a_controller.get());
			}
		}

		m_controller = a_controller;
	}

	void EngineExtensions::Initialize(
		const std::shared_ptr<Controller>& a_controller)
	{
		if (!m_Instance)
		{
			m_Instance = std::make_unique<EngineExtensions>(a_controller);
		}
	}

	auto EngineExtensions::ValidateMemory(
		const Config& a_config) -> MemoryValidationFlags
	{
		using namespace Patching;

		MemoryValidationFlags result(MemoryValidationFlags::kNone);

		constexpr std::uint8_t d_memCall5[]{ 0xE8 };

		if (!validate_mem(m_getShieldWeaponSlotNode_a, d_memCall5))
		{
			result |= MemoryValidationFlags::kWeaponLeftAttach;
		}

		if (!validate_mem(m_getStaffSlotNode_a, d_memCall5))
		{
			result |= MemoryValidationFlags::kStaffAttach;
		}

		if (!validate_mem(m_scbGet_a, d_memCall5))
		{
			result |= MemoryValidationFlags::kScabbardGet;
		}

		if (a_config.m_shield.IsEnabled())
		{
			if (!validate_mem(m_getShieldArmorSlotNode_a, d_memCall5))
			{
				result |= MemoryValidationFlags::kShieldAttach;
			}
		}

		if ((a_config.m_shieldHideFlags & Data::Flags::kEnabled) != Data::Flags::kNone)
		{
			if (IAL::IsAE())
			{
				constexpr std::uint8_t d_hideShield[]{ 0x4C, 0x8B, 0x0A, 0x4D, 0x85, 0xC9, 0x74, 0x4F };
				if (!validate_mem(m_hideShield_a, d_hideShield))
				{
					result |= MemoryValidationFlags::kDisableShieldHideOnSit;
				}
			}
			else
			{
				constexpr std::uint8_t d_hideShield[]{ 0x4C, 0x8B, 0x0A, 0x48, 0x8B, 0x81, 0xF0, 0x01, 0x00, 0x00 };
				if (!validate_mem(m_hideShield_a, d_hideShield))
				{
					result |= MemoryValidationFlags::kDisableShieldHideOnSit;
				}
			}
		}

		if (a_config.m_scb)
		{
			if (IAL::IsAE())
			{
				constexpr std::uint8_t d_scbAttach[]{ 0x80, 0x7D, 0x6F, 0x00, 0x75, 0x1E, 0x48, 0x8B, 0x06 };
				if (!validate_mem(m_scbAttach_a, d_scbAttach))
				{
					result |= MemoryValidationFlags::kScabbardAttach;
				}

				constexpr std::uint8_t d_scbDetach[]{ 0x0F, 0x84, 0xB1, 0x00, 0x00, 0x00 };
				if (!validate_mem(m_scbDetach_a, d_scbDetach))
				{
					result |= MemoryValidationFlags::kScabbardDetach;
				}
			}
			else
			{
				constexpr std::uint8_t d_scbAttach[]{ 0x80, 0x7D, 0x6F, 0x00, 0x75, 0x1E };
				if (!validate_mem(m_scbAttach_a, d_scbAttach))
				{
					result |= MemoryValidationFlags::kScabbardAttach;
				}

				constexpr std::uint8_t d_scbDetach[]{ 0x0F, 0x84, 0xB3, 0x00, 0x00, 0x00 };
				if (!validate_mem(m_scbDetach_a, d_scbDetach))
				{
					result |= MemoryValidationFlags::kScabbardDetach;
				}
			}
		}

		return result;
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
				je(cont);  // skip if not left

				mov(rcx, ptr[rbp + 0x77]);
				mov(rcx, ptr[rcx]);        // Biped
				mov(r8, ptr[rbp + 0x5F]);  // root

				if (IAL::IsAE())
				{
					mov(edx, r15d);  // slot

					call(ptr[rip + callLabel]);

					test(rax, rax);
					je(skip);
					mov(rsi, rax);  // rsi: not used after scb is attached
				}
				else
				{
					mov(edx, r14d);  // slot

					push(rax);
					sub(rsp, 0x28);

					call(ptr[rip + callLabel]);
					mov(rdx, rax);

					add(rsp, 0x28);
					pop(rax);

					test(rdx, rdx);
					je(skip);
					mov(rsi, rdx);  // rsi: not used after scb is attached
				}

				L(cont);
				jmp(ptr[rip + exitContinue]);

				L(skip);
				jmp(ptr[rip + exitSkip]);

				L(exitContinue);
				dq(targetAddr + 0x6);

				L(exitSkip);
				dq(targetAddr + 0x24);

				L(callLabel);
				dq(std::uintptr_t(GetScbAttachmentNode_Hook));
			}
		};

		LogPatchBegin(__FUNCTION__);
		{
			Assembly code(m_scbAttach_a);
			ISKSE::GetBranchTrampoline().Write6Branch(m_scbAttach_a, code.get());
		}
		LogPatchEnd(__FUNCTION__);
	}

	void EngineExtensions::Patch_SCB_Detach()
	{
		struct Assembly : JITASM
		{
			Assembly(std::uintptr_t targetAddr) :
				JITASM(ISKSE::GetLocalTrampoline())
			{
				Xbyak::Label callLabel;
				Xbyak::Label exitIsWeapon;
				Xbyak::Label exitIsShield;

				Xbyak::Label isWeaponSlot;
				Xbyak::Label ok;

				jne(isWeaponSlot);

				//mov(rcx, ptr[rbp - 0x20]); // reference - still held, should be safe to use
				mov(rcx, ptr[rdi]);  // form
				//mov(rdx, r12); // attachment node
				mov(rdx, r13);  // Biped
				call(ptr[rip + callLabel]);
				test(rax, rax);
				jne(ok);
				jmp(ptr[rip + exitIsShield]);

				L(ok);
				mov(r12, rax);

				L(isWeaponSlot);
				jmp(ptr[rip + exitIsWeapon]);

				L(exitIsShield);
				dq(targetAddr + (IAL::IsAE() ? 0xB7 : 0xB9));

				L(exitIsWeapon);
				dq(targetAddr + 0x6);

				L(callLabel);
				dq(std::uintptr_t(GetScbAttachmentNode_Cleanup_Hook));
			}
		};

		LogPatchBegin(__FUNCTION__);
		{
			Assembly code(m_scbDetach_a);
			ISKSE::GetBranchTrampoline().Write6Branch(m_scbDetach_a, code.get());
		}
		LogPatchEnd(__FUNCTION__);
	}

	void EngineExtensions::Patch_SCB_Get()
	{
		struct Assembly : JITASM
		{
			Assembly(std::uintptr_t targetAddr) :
				JITASM(ISKSE::GetLocalTrampoline())
			{
				Xbyak::Label retnLabel;
				Xbyak::Label callLabel;

				if (IAL::IsAE())
				{
					mov(r8d, 0x1);
					xor_(r9d, r9d);
					mov(rax, ptr[rbp - 0x41]);
					cmp(ptr[rbp - 0x39], rax);
					cmove(r9d, r8d);  // is 1p
				}
				else
				{
					mov(r9b, byte[rbp - 0x51]);  // is 1p
				}

				mov(r8b, byte[rbp + 0x6F]);  // is left weapon

				call(ptr[rip + callLabel]);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(targetAddr + 0x5);

				L(callLabel);
				dq(std::uintptr_t(GetScabbardNode_Hook));
			}
		};

		LogPatchBegin(__FUNCTION__);
		{
			Assembly code(m_scbGet_a);
			ISKSE::GetBranchTrampoline().Write5Branch(m_scbGet_a, code.get());
		}
		LogPatchEnd(__FUNCTION__);
	}

	// right staff and shield slot (weapon)
	void EngineExtensions::Patch_WeaponObjects_Attach()
	{
		struct Assembly : JITASM
		{
			Assembly(
				std::uintptr_t a_targetAddr,
				std::uintptr_t a_callAddr,
				std::uintptr_t a_retnNoHiddenOffset) :
				JITASM(ISKSE::GetLocalTrampoline())
			{
				Xbyak::Label callLabel;
				Xbyak::Label retnLabel;
				Xbyak::Label retnNoHiddenLabel;

				Xbyak::Label skipHide;

				sub(rsp, 0x40);

				if (IAL::IsAE())
				{
					mov(r9d, 0x1);
					xor_(r8d, r8d);
					mov(rax, ptr[rbp - 0x41]);
					cmp(ptr[rbp - 0x39], rax);
					cmove(r8d, r9d);
					mov(byte[rsp + 0x20], r8b);  // is 1p

					mov(r9d, r15d);  // slot
				}
				else
				{
					mov(al, byte[rbp - 0x51]);
					mov(byte[rsp + 0x20], al);  // is 1p

					mov(r9d, r14d);  // slot
				}

				mov(r8, ptr[rbp + 0x77]);
				mov(r8, ptr[r8]);  // Biped

				lea(rax, ptr[rsp + 0x30]);
				mov(ptr[rsp + 0x28], rax);  // skip hide bool

				call(ptr[rip + callLabel]);
				mov(dl, byte[rsp + 0x30]);

				add(rsp, 0x40);

				test(dl, dl);
				jne(skipHide);

				jmp(ptr[rip + retnLabel]);

				L(skipHide);

				mov(rsi, rax);                        // store attachment node

				jmp(ptr[rip + retnNoHiddenLabel]);

				L(retnLabel);
				dq(a_targetAddr + 0x5);

				L(retnNoHiddenLabel);
				dq(a_targetAddr + a_retnNoHiddenOffset);

				L(callLabel);
				dq(a_callAddr);
			}
		};

		LogPatchBegin(__FUNCTION__);
		{
			{
				Assembly code(m_getShieldWeaponSlotNode_a, std::uintptr_t(GetWeaponShieldSlotNode_Hook), IAL::IsAE() ? 0x28 : 0x26);
				ISKSE::GetBranchTrampoline().Write5Branch(m_getShieldWeaponSlotNode_a, code.get());
			}

			{
				Assembly code(m_getStaffSlotNode_a, std::uintptr_t(GetWeaponStaffSlotNode_Hook), IAL::IsAE() ? 0x24 : 0x26);
				ISKSE::GetBranchTrampoline().Write5Branch(m_getStaffSlotNode_a, code.get());
			}
		}
		LogPatchEnd(__FUNCTION__);
	}

	void EngineExtensions::Patch_ShieldObject_Attach()
	{
		struct Assembly : JITASM
		{
			Assembly(
				std::uintptr_t a_targetAddr) :
				JITASM(ISKSE::GetLocalTrampoline())
			{
				Xbyak::Label callLabel;
				Xbyak::Label retnLabel;

				sub(rsp, 0x30);

				if (IAL::IsAE())
				{
					mov(r9d, 0x1);
					xor_(r8d, r8d);
					mov(rax, ptr[rbp - 0x41]);
					cmp(ptr[rbp - 0x39], rax);
					cmove(r8d, r9d);
					mov(byte[rsp + 0x20], r8b);  // is 1p

					mov(r9d, r15d);  // slot
				}
				else
				{
					mov(al, byte[rbp - 0x51]);
					mov(byte[rsp + 0x20], al);  // is 1p

					mov(r9d, r14d);  // slot
				}

				mov(r8, ptr[rbp + 0x77]);
				mov(r8, ptr[r8]);  // Biped

				call(ptr[rip + callLabel]);

				add(rsp, 0x30);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(a_targetAddr + 0x5);

				L(callLabel);
				dq(std::uintptr_t(GetShieldArmorSlotNode_Hook));
			}
		};

		LogPatchBegin(__FUNCTION__);
		{
			Assembly code(m_getShieldArmorSlotNode_a);
			ISKSE::GetBranchTrampoline().Write5Branch(m_getShieldArmorSlotNode_a, code.get());
		}
		LogPatchEnd(__FUNCTION__);
	}

	void EngineExtensions::Patch_DisableShieldHideOnSit(
		const Config& a_config)
	{
		struct Assembly : JITASM
		{
			Assembly(
				std::uintptr_t a_targetAddr) :
				JITASM(ISKSE::GetLocalTrampoline())
			{
				Xbyak::Label exitContinue;
				Xbyak::Label exitSkip;
				Xbyak::Label callLabel;

				Xbyak::Label skip;
				Xbyak::Label cont;

				test(r8b, r8b);  // bool: true = hide, false = show
				je(cont);

				push(rcx);
				push(rdx);
				push(r8);
				sub(rsp, 0x28);

				call(ptr[rip + callLabel]);

				add(rsp, 0x28);
				pop(r8);
				pop(rdx);
				pop(rcx);

				test(al, al);
				jne(skip);

				L(cont);
				mov(r9, ptr[rdx]);  // Biped
				if (IAL::IsAE())
				{
					test(r9, r9);
				}
				else
				{
					mov(rax, ptr[rcx + 0x1F0]);  // TESRace*
				}
				jmp(ptr[rip + exitContinue]);

				L(skip);
				jmp(ptr[rip + exitSkip]);

				L(exitContinue);
				dq(a_targetAddr + (IAL::IsAE() ? 0x6 : 0xA));

				L(exitSkip);
				dq(a_targetAddr + (IAL::IsAE() ? 0x57 : 0x53));

				L(callLabel);
				dq(std::uintptr_t(ShouldBlockShieldHide));
			}
		};

		LogPatchBegin(__FUNCTION__);
		{
			Assembly code(m_hideShield_a);
			ISKSE::GetBranchTrampoline().Write6Branch(m_hideShield_a, code.get());
		}
		LogPatchEnd(__FUNCTION__);
	}

	bool EngineExtensions::Hook_TESObjectWEAP_SetEquipSlot()
	{
		bool result = VTable::Detour2(
			m_vtbl_TESObjectWEAP,
			0x86 + 0x5,
			TESObjectWEAP_SetEquipSlot_Hook,
			std::addressof(m_TESObjectWEAP_SetEquipSlot_o));

		if (result)
		{
			Message("[Hook] TESObjectWEAP::SetEquipSlot");
		}
		else
		{
			Error("%s: FAILED", __FUNCTION__);
		}
		return result;
	}

	bool EngineExtensions::Patch_ShieldHandWorkaround()
	{
		if (!Hook::GetDst5<0xE8>(
				m_unk140609D50_BShkbAnimationGraph_SetGraphVariableInt_a,
				m_BShkbAnimationGraph_SetGraphVariableInt_o))
		{
			return false;
		}

		if (!Hook::GetDst5<0xE8>(
				m_unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_a,
				m_unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_o))
		{
			return false;
		}

		if (!Hook::GetDst5<0xE8>(
				m_unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_a,
				m_unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_o))
		{
			return false;
		}

		enum class HookTarget
		{
			k1,
			k2,
			k3
		};

		struct Assembly : JITASM
		{
			Assembly(HookTarget a_target) :
				JITASM(ISKSE::GetLocalTrampoline())
			{
				Xbyak::Label retnLabel;
				Xbyak::Label callLabel;

				std::uintptr_t targetAddr;
				std::uintptr_t callAddr;

				switch (a_target)
				{
				case HookTarget::k1:
					targetAddr = m_unk140609D50_BShkbAnimationGraph_SetGraphVariableInt_a;
					callAddr = std::uintptr_t(Unk140609D50_BShkbAnimationGraph_SetGraphVariableInt_Hook);
					lea(r9, ptr[rbp - 0x38]);
					break;
				case HookTarget::k2:
					targetAddr = m_unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_a;
					callAddr = std::uintptr_t(Unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_Hook);
					mov(r9, rsi);
					break;
				case HookTarget::k3:
					targetAddr = m_unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_a;
					callAddr = std::uintptr_t(Unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_Hook);
					lea(r9, ptr[r15 - 0xB8]);
					break;
				}

				call(ptr[rip + callLabel]);
				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(targetAddr + 0x5);

				L(callLabel);
				dq(callAddr);
			}
		};

		LogPatchBegin("Unk140609D50");
		{
			Assembly code(HookTarget::k1);
			ISKSE::GetBranchTrampoline().Write5Branch(
				m_unk140609D50_BShkbAnimationGraph_SetGraphVariableInt_a,
				code.get());
		}
		LogPatchEnd("Unk140609D50");

		LogPatchBegin("Unk1406097C0");
		{
			Assembly code(HookTarget::k2);
			ISKSE::GetBranchTrampoline().Write5Branch(
				m_unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_a,
				code.get());
		}
		LogPatchEnd("Unk1406097C0");

		LogPatchBegin("Unk140634D20");
		{
			Assembly code(HookTarget::k3);
			ISKSE::GetBranchTrampoline().Write5Branch(
				m_unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_a,
				code.get());
		}
		LogPatchEnd("Unk140634D20");

		return true;
	}

	bool EngineExtensions::Unk140609D50_BShkbAnimationGraph_SetGraphVariableInt_Hook(
		BShkbAnimationGraph* a_graph,
		const BSFixedString& a_name,
		std::int32_t a_value,
		Actor* a_actor)
	{
		auto controller = m_Instance->m_controller.get();

		if (a_value == 10 &&
		    controller->IsShieldEnabled(a_actor) &&
		    controller->GetShieldOnBackSwitch(a_actor) &&
		    (controller->GetConfig().m_shwForceIfDrawn ||
		     !a_actor->actorState.IsWeaponDrawn()) &&
		    Common::IsShieldEquipped(a_actor))
		{
			a_value = 0;
		}

		return m_Instance->m_BShkbAnimationGraph_SetGraphVariableInt_o(a_graph, a_name, a_value);
	}

	std::uint32_t EngineExtensions::Unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_Hook(
		IAnimationGraphManagerHolder* a_holder,
		const BSFixedString& a_name,
		std::int32_t a_value,
		Actor* a_actor)
	{
		auto controller = m_Instance->m_controller.get();

		if (a_value == 10 &&
		    controller->IsShieldEnabled(a_actor) &&
		    controller->GetShieldOnBackSwitch(a_actor) &&
		    (controller->GetConfig().m_shwForceIfDrawn ||
		     !a_actor->actorState.IsWeaponDrawn()) &&
		    Common::IsShieldEquipped(a_actor))
		{
			a_value = 0;
		}

		return m_Instance->m_unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_o(a_holder, a_name, a_value);
	}

	std::uint32_t EngineExtensions::Unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_Hook(
		IAnimationGraphManagerHolder* a_holder,
		const BSFixedString& a_name,
		std::int32_t a_value,
		Actor* a_actor)
	{
		auto controller = m_Instance->m_controller.get();

		if ((a_value == 0 || a_value == 10) &&
		    controller->IsShieldEnabled(a_actor) &&
		    controller->GetShieldOnBackSwitch(a_actor) &&
		    Common::IsShieldEquipped(a_actor))
		{
			a_holder->SetVariableOnGraphsInt(
				controller->GetStringHolder()->m_iLeftHandType,
				a_value);
		}

		return m_Instance->m_unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_o(a_holder, a_name, a_value);
	}

	void EngineExtensions::TESObjectWEAP_SetEquipSlot_Hook(BGSEquipType* a_this, BGSEquipSlot* a_slot)
	{
		// this just writes BGSEquipSlot* @rcx+8, but we call here on the off chance something else hooked it
		m_Instance->m_TESObjectWEAP_SetEquipSlot_o(a_this, a_slot);

		OnSetEquipSlot evn{};
		m_Instance->m_dispatchers.m_setEquipSlot.SendEvent(evn);
	}

	NiNode* EngineExtensions::GetScbAttachmentNode_Hook(
		Biped* a_biped,
		std::uint32_t a_bipedSlot,
		NiNode* a_root)
	{
		// these checks should never fail
		if (!a_root || !a_biped || a_bipedSlot == 0xFFFFFFFF)
		{
			return nullptr;
		}

		NiPointer<TESObjectREFR> ref;
		if (!a_biped->handle.Lookup(ref))
		{
			return nullptr;
		}

		auto actor = ref->As<Actor>();
		if (!actor)
		{
			return nullptr;
		}

		auto form = a_biped->objects[a_bipedSlot].item;
		if (!form)
		{
			return nullptr;
		}

		auto weapon = form->As<TESObjectWEAP>();
		if (!weapon)
		{
			return nullptr;
		}

		return m_Instance->m_controller->GetScbAttachmentNode(
			actor,
			weapon,
			a_root,
			false // this hook won't run for 1p
		);
	}

	NiNode* EngineExtensions::GetScbAttachmentNode_Cleanup_Hook(
		TESForm* a_form,
		Biped* a_biped)
	{
		if (!a_form || !a_biped)
		{
			return nullptr;
		}

		auto weapon = a_form->As<TESObjectWEAP>();
		if (!weapon)
		{
			return nullptr;
		}

		auto root = a_biped->root;
		if (!root)
		{
			return nullptr;
		}

		auto name = m_Instance->m_controller->GetScbAttachmentNodeName(
			root,
			weapon);

		if (!name)
		{
			return nullptr;
		}

		auto object = GetNodeByName(root, *name, true);
		if (!object)
		{
			return nullptr;
		}

		return object->GetAsNiNode();
	}

	NiAVObject* EngineExtensions::GetWeaponShieldSlotNode_Hook(
		NiNode* a_root,
		const BSFixedString& a_nodeName,
		Biped* a_biped,
		std::uint32_t a_bipedSlot,
		bool a_is1p,
		bool& a_skipHide)
	{
		auto str = m_Instance->GetWeaponNodeName(a_biped, a_bipedSlot, a_is1p, true);

		if (!str)
		{
			a_skipHide = false;
			str = std::addressof(a_nodeName);
		}
		else
		{
			a_skipHide = true;
		}

		return GetNodeByName(a_root, *str, true);
	}

	NiAVObject* EngineExtensions::GetWeaponStaffSlotNode_Hook(
		NiNode* a_root,
		const BSFixedString& a_nodeName,
		Biped* a_biped,
		std::uint32_t a_bipedSlot,
		bool a_is1p,
		bool& a_skipHidden)
	{
		auto str = m_Instance->GetWeaponNodeName(a_biped, a_bipedSlot, a_is1p, false);

		if (!str)
		{
			a_skipHidden = false;
			str = std::addressof(a_nodeName);
		}
		else
		{
			a_skipHidden = true;
		}

		return GetNodeByName(a_root, *str, true);
	}

	NiAVObject* EngineExtensions::GetShieldArmorSlotNode_Hook(
		NiNode* a_root,
		const BSFixedString& a_nodeName,
		Biped* a_biped,
		std::uint32_t a_bipedSlot,
		bool a_is1p)
	{
		if (a_biped && a_bipedSlot != 0xFFFFFFFF)
		{
			NiPointer<TESObjectREFR> ref;
			if (a_biped->handle.Lookup(ref))
			{
				if (auto actor = ref->As<Actor>(); actor)
				{
					if (auto form = a_biped->objects[a_bipedSlot].item; form)
					{
						if (auto armor = form->As<TESObjectARMO>(); armor)
						{
							if (Controller::GetShieldBipedObject(actor) == a_bipedSlot)
							{
								if (auto str = m_Instance->m_controller->GetShieldAttachmentNodeName(actor, armor, a_is1p))
								{
									return GetNodeByName(a_root, *str, true);
								}
							}
						}
					}
				}
			}
		}

		return GetNodeByName(a_root, a_nodeName, true);
	}

	NiAVObject* EngineExtensions::GetScabbardNode_Hook(
		NiNode* a_node,
		const BSFixedString& a_nodeName,  // Scb
		bool a_left,
		bool a_is1p)
	{
		auto stringHolder = m_Instance->m_controller->GetStringHolder();

		NiPointer scbNode = GetNodeByName(a_node, a_nodeName, true);
		NiPointer scbLeftNode = GetNodeByName(a_node, stringHolder->m_scbLeft, true);

		auto& config = m_Instance->m_controller->GetConfig();

		if (!a_left ||
		    a_is1p ||
		    !config.m_scb ||
		    !config.m_scbCustom)
		{
			if (scbLeftNode && scbLeftNode->m_parent)
			{
				scbLeftNode->m_parent->RemoveChild(scbLeftNode);
				ShrinkChildrenToSize(a_node);
			}

			return scbNode;
		}

		if (!scbLeftNode)
		{
			return scbNode;
		}

		if (scbNode && scbNode->m_parent)
		{
			scbNode->m_parent->RemoveChild(scbNode);
			ShrinkChildrenToSize(a_node);
		}

		Node::SetVisible(scbLeftNode);

		return scbLeftNode;
	}

	bool EngineExtensions::ShouldBlockShieldHide(
		Actor* a_actor)
	{
		return m_Instance->m_controller->ShouldBlockShieldHide(a_actor);
	}

	const BSFixedString* EngineExtensions::GetWeaponNodeName(
		Biped* a_biped,
		std::uint32_t a_bipedSlot,
		bool a_is1p,
		bool a_left)
	{
		if (!a_biped || a_bipedSlot == 0xFFFFFFFF)
		{
			return nullptr;
		}

		NiPointer<TESObjectREFR> ref;
		if (!a_biped->handle.Lookup(ref))
		{
			return nullptr;
		}

		auto actor = ref->As<Actor>();
		if (!actor)
		{
			return nullptr;
		}

		auto form = a_biped->objects[a_bipedSlot].item;
		if (!form)
		{
			return nullptr;
		}

		auto weapon = form->As<TESObjectWEAP>();
		if (!weapon)
		{
			return nullptr;
		}

		return m_controller->GetWeaponAttachmentNodeName(actor, weapon, a_is1p, a_left);
	}

}