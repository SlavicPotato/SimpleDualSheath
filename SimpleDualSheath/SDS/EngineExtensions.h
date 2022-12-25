#pragma once

#include "Controller.h"

#include "Events/CreateArmorNodeEvent.h"
#include "Events/CreateWeaponNodesEvent.h"
#include "Events/OnSetEquipSlot.h"

namespace SDS
{
	class BShkbAnimationGraph;

	class EngineExtensions :
		ILog
	{
	public:
		EngineExtensions(const std::shared_ptr<Controller>& a_controller);

		EngineExtensions(const EngineExtensions&) = delete;
		EngineExtensions(EngineExtensions&&)      = delete;
		EngineExtensions& operator=(const EngineExtensions&) = delete;
		EngineExtensions& operator=(EngineExtensions&&) = delete;

		enum class MemoryValidationFlags : std::uint8_t
		{
			kNone = 0,

			kWeaponLeftAttach       = 1ui8 << 0,
			kStaffAttach            = 1ui8 << 1,
			kShieldAttach           = 1ui8 << 2,
			kDisableShieldHideOnSit = 1ui8 << 3,
			kScabbardAttach         = 1ui8 << 4,
			kScabbardDetach         = 1ui8 << 5,
			kScabbardGet            = 1ui8 << 6,
		};

		static void                  Initialize(const std::shared_ptr<Controller>& a_controller);
		static MemoryValidationFlags ValidateMemory(const Config& a_config);

		inline static auto GetSingleton() noexcept
		{
			return m_Instance.get();
		}

		inline constexpr auto& GetEventDispatchers() noexcept
		{
			return m_dispatchers;
		}

		FN_NAMEPROC("EngineExtensions");

	private:
		void Patch_SCB_Attach();
		void Patch_SCB_Detach();
		void Patch_SCB_Get();
		void Patch_WeaponObjects_Attach();
		void Patch_ShieldObject_Attach();
		void Patch_DisableShieldHideOnSit();
		bool Hook_TESObjectWEAP_SetEquipSlot();
		bool Patch_ShieldHandWorkaround();

		static_assert(std::is_same_v<std::underlying_type_t<BIPED_OBJECT>, std::uint32_t>);

		static NiNode*     GetScbAttachmentNode_Hook(Biped* a_biped, BIPED_OBJECT a_bipedSlot, NiNode* a_root);
		static NiNode*     GetScbAttachmentNode_Cleanup_Hook(TESForm* a_form, Biped* a_biped);
		static NiAVObject* GetWeaponShieldSlotNode_Hook(NiNode* a_root, const BSFixedString& a_nodeName, Biped* a_biped, BIPED_OBJECT a_bipedSlot, bool a_is1p, bool& a_skipHide);
		static NiAVObject* GetWeaponStaffSlotNode_Hook(NiNode* a_root, const BSFixedString& a_nodeName, Biped* a_biped, BIPED_OBJECT a_bipedSlot, bool a_is1p, bool& a_skipHide);
		static NiAVObject* GetShieldArmorSlotNode_Hook(NiNode* a_root, const BSFixedString& a_nodeName, Biped* a_biped, BIPED_OBJECT a_bipedSlot, bool a_is1p);
		static NiAVObject* GetScabbardNode_Hook(NiNode* a_object, const BSFixedString& a_nodeName, bool a_left, bool a_is1p);
		static void        TESObjectWEAP_SetEquipSlot_Hook(BGSEquipType* a_this, BGSEquipSlot* a_slot);

		static bool          Unk140609D50_BShkbAnimationGraph_SetGraphVariableInt_Hook(BShkbAnimationGraph* a_graph, const BSFixedString& a_name, std::int32_t a_value, Actor* a_actor);
		static std::uint32_t Unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_Hook(RE::IAnimationGraphManagerHolder* a_holder, const BSFixedString& a_name, std::int32_t a_value, Actor* a_actor);
		static std::uint32_t Unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_Hook(RE::IAnimationGraphManagerHolder* a_holder, const BSFixedString& a_name, std::int32_t a_value, Actor* a_actor);

		using BShkbAnimationGraph_SetGraphVariableInt_t             = bool (*)(BShkbAnimationGraph* a_graph, const BSFixedString& a_name, std::int32_t a_value);
		using IAnimationGraphManagerHolder_SetVariableOnGraphsInt_t = std::uint32_t (*)(RE::IAnimationGraphManagerHolder* a_holder, const BSFixedString& a_name, std::int32_t a_value);

		static bool          ShouldBlockShieldHide(Actor* a_actor);
		const BSFixedString* GetWeaponNodeName(Biped* a_biped, BIPED_OBJECT a_bipedSlot, bool a_is1p, bool a_left);

		decltype(&TESObjectWEAP_SetEquipSlot_Hook)            m_TESObjectWEAP_SetEquipSlot_o;
		BShkbAnimationGraph_SetGraphVariableInt_t             m_BShkbAnimationGraph_SetGraphVariableInt_o;
		IAnimationGraphManagerHolder_SetVariableOnGraphsInt_t m_unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_o;
		IAnimationGraphManagerHolder_SetVariableOnGraphsInt_t m_unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_o;

		using fGetNodeByName_t = NiAVObject* (*)(NiNode* a_root, const BSFixedString& a_name, bool a_unk);
		using fUnk1401CDB30_t  = NiAVObject* (*)(NiNode*);

		struct
		{
			::Events::EventDispatcher<Events::OnSetEquipSlot> m_setEquipSlot;
		} m_dispatchers;

		std::shared_ptr<Controller> m_controller;

		inline static auto m_scbAttach_a               = IAL::Address<std::uintptr_t>(15569, 15746, 0x3A3, 0x3BA);
		inline static auto m_scbGet_a                  = IAL::Address<std::uintptr_t>(15569, 15746, 0x383, 0x396);
		inline static auto m_getShieldWeaponSlotNode_a = IAL::Address<std::uintptr_t>(15569, 15746, 0x1D1, 0x1C3);
		inline static auto m_getStaffSlotNode_a        = IAL::Address<std::uintptr_t>(15569, 15746, 0x223, 0x217);
		inline static auto m_getShieldArmorSlotNode_a  = IAL::Address<std::uintptr_t>(15569, 15746, 0x260, 0x255);
		inline static auto m_scbDetach_a               = IAL::Address<std::uintptr_t>(15496, 15661, 0x1A3, 0x1A5);
		inline static auto m_hideShield_a              = IAL::Address<std::uintptr_t>(36580, 37584, 0x6, 0x6);  // does other stuff but we don't care here
		inline static auto m_vtbl_TESObjectWEAP        = IAL::Address<std::uintptr_t>(234396, 189786);

		inline static auto GetNodeByName = IAL::Address<fGetNodeByName_t>(74481, 76207);
		inline static auto ShrinkToSize  = IAL::Address<fUnk1401CDB30_t>(15571, 15748);

		inline static auto m_unk140609D50_BShkbAnimationGraph_SetGraphVariableInt_a             = IAL::Address<std::uintptr_t>(36957, 37982, 0x1DA, 0x19E);  // load (iLeftHandType), rbp - 38 = Actor (BShkbAnimationGraph::SetGraphVariableInt)
		inline static auto m_unk1406097C0_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_a = IAL::Address<std::uintptr_t>(36949, 37974, 0x48, 0x48);    // equip (iLeftHandType), rsi = Actor (IAnimationGraphManagerHolder::SetVariableOnGraphsInt)
		inline static auto m_unk140634D20_IAnimationGraphManagerHolder_SetVariableOnGraphsInt_a = IAL::Address<std::uintptr_t>(37866, 38821, 0x17B, 0x13F);  // draw (iLeftHandEquipped), r15 - B8 = Actor (IAnimationGraphManagerHolder::SetVariableOnGraphsInt)

		static std::unique_ptr<EngineExtensions> m_Instance;
	};

	DEFINE_ENUM_CLASS_BITWISE(EngineExtensions::MemoryValidationFlags);
}