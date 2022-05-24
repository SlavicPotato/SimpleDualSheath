#include "pch.h"

#include "Controller.h"

#include "Util/Common.h"
#include "Util/Node.h"

#include <ext/GameCommon.h>
#include <ext/Node.h>

namespace SDS
{
	using namespace Util::Common;
	using namespace Util::Node;
	using namespace ::Util::Node;
	using namespace Data;

	Controller::Controller(
		const Config& a_conf) :
		m_conf(a_conf),
		m_shieldOnBackSwitch(1)
	{
	}

	void Controller::InitializeData()
	{
		m_strings = std::make_shared<StringHolder>();

		m_data = std::make_unique<WeaponData>();

		m_data->Create(WEAPON_TYPE::kOneHandSword, StringHolder::NINODE_SWORD, StringHolder::NINODE_SWORD_LEFT, m_conf.m_sword);
		m_data->Create(WEAPON_TYPE::kOneHandAxe, StringHolder::NINODE_AXE, StringHolder::NINODE_AXE_LEFT, m_conf.m_axe);
		m_data->Create(WEAPON_TYPE::kOneHandMace, StringHolder::NINODE_MACE, StringHolder::NINODE_MACE_LEFT, m_conf.m_mace);
		m_data->Create(WEAPON_TYPE::kOneHandDagger, StringHolder::NINODE_DAGGER, StringHolder::NINODE_DAGGER_LEFT, m_conf.m_dagger);
		m_data->Create(WEAPON_TYPE::kStaff, StringHolder::NINODE_STAFF, StringHolder::NINODE_STAFF_LEFT, m_conf.m_staff);
		m_data->Create(WEAPON_TYPE::kTwoHandSword, StringHolder::NINODE_WEAPON_BACK, StringHolder::NINODE_SWORD_ON_BACK_LEFT, m_conf.m_2hSword);
		m_data->Create(WEAPON_TYPE::kTwoHandAxe, StringHolder::NINODE_WEAPON_BACK, StringHolder::NINODE_AXE_ON_BACK_LEFT, m_conf.m_2hAxe);

		if (!m_conf.m_shield.m_sheathNode.empty())
		{
			m_strings->m_shieldSheathNode = m_conf.m_shield.m_sheathNode.c_str();
		}

#ifdef _SDS_UNUSED
		m_nodeOverride = std::make_unique<NodeOverride>(m_strings);
#endif
	}

	bool Controller::GetParentNodes(
		const Data::Weapon* a_entry,
		NiNode*             a_root,
		bool                a_left,
		NiNode*&            a_sheathedNode,
		NiNode*&            a_drawnNode) const
	{
		auto nodea = a_entry->GetNode(a_root, a_left);
		if (!nodea)
		{
			return false;
		}

		auto nodeb = FindNode(
			a_root,
			a_left ?
				m_strings->m_shield :
                m_strings->m_weapon);

		if (!nodeb)
		{
			return false;
		}

		a_sheathedNode = nodea;
		a_drawnNode    = nodeb;

		return true;
	}

	bool Controller::GetIsDrawn(
		Actor*     a_actor,
		DrawnState a_state)
	{
		switch (a_state)
		{
		case DrawnState::Drawn:
			return true;
		case DrawnState::Sheathed:
			return false;
		default:
			return a_actor->IsWeaponDrawn();
		};
	}

	BIPED_OBJECT Controller::GetShieldBipedObject(
		Actor* a_actor)
	{
		if (auto baseForm = a_actor->baseForm)
		{
			if (auto npc = baseForm->As<TESNPC>())
			{
				if (auto race = npc->race)
				{
					return race->data.shieldObject;
				}
			}
		}

		return BIPED_OBJECT::kNone;
	}

	void Controller::ProcessEquippedWeapon(
		Actor*             a_actor,
		const NiRootNodes& a_roots,
		TESObjectWEAP*     a_weapon,
		bool               a_drawn,
		bool               a_left) const
	{
		auto entry = m_data->Get(a_actor, a_weapon, a_left);
		if (!entry)
		{
			return;
		}

		char buf[MAX_PATH];
		a_weapon->GetNodeName(buf);

		BSFixedString weaponNodeName(buf);

		for (std::uint32_t i = 0; i < std::size(a_roots.m_nodes); i++)
		{
			auto& root = a_roots.m_nodes[i];

			if (!root)
			{
				continue;
			}

			if (i == 1 && !entry->FirstPerson())
			{
				continue;
			}

			NiNode *sheathedNode, *drawnNode;
			if (!GetParentNodes(entry, root, a_left, sheathedNode, drawnNode))
			{
				continue;
			}

			auto sourceNode = a_drawn ? sheathedNode : drawnNode;
			auto targetNode = a_drawn ? drawnNode : sheathedNode;

			if (auto w1 = FindChildObject(sourceNode, weaponNodeName))
			{
				AttachToNode(w1, targetNode);
				SetVisible(w1);
			}
			else if (auto w2 = FindChildObject(targetNode, weaponNodeName))
			{
				SetVisible(w2);
			}
		}
	}

	void Controller::ProcessWeaponDrawnChange(
		Actor* a_actor,
		bool   a_drawn) const
	{
		auto pm = a_actor->processManager;
		if (!pm)
		{
			return;
		}

		NiRootNodes roots(a_actor);
		roots.GetNPCRoots(m_strings->m_npcroot);

		auto form = pm->equippedObject[ActorProcessManager::kEquippedHand_Left];
		if (form)
		{
			if (form->IsWeapon())
			{
				ProcessEquippedWeapon(a_actor, roots, static_cast<TESObjectWEAP*>(form), a_drawn, true);
			}
			else if (form->IsArmor())
			{
				auto armor = static_cast<TESObjectARMO*>(form);
				if (armor->IsShield() && m_conf.m_shield.IsEnabled())
				{
					ProcessEquippedShield(a_actor, roots, a_drawn, GetShieldOnBackSwitch(a_actor));
				}
			}
		}

		form = pm->equippedObject[ActorProcessManager::kEquippedHand_Right];
		if (form)
		{
			if (auto weapon = form->As<TESObjectWEAP>())
			{
				ProcessEquippedWeapon(a_actor, roots, weapon, a_drawn, false);
			}
		}
	}

	void Controller::QueueProcessWeaponDrawnChange(
		TESObjectREFR* a_actor,
		DrawnState     a_drawnState) const
	{
		ITaskPool::QueueLoadedActorTask(
			a_actor,
			[this, a_drawnState](
				Actor* a_actor,
				Game::ActorHandle) {
				ProcessWeaponDrawnChange(
					a_actor,
					GetIsDrawn(a_actor, a_drawnState));
			});
	}

	bool Controller::IsShieldEnabled(Actor* a_actor) const
	{
		return a_actor == *g_thePlayer ?
		           m_conf.m_shield.m_flags.test(Flags::kPlayer) :
                   m_conf.m_shield.m_flags.test(Flags::kNPC);
	}

	bool Controller::IsShieldEnabled(bool a_player) const
	{
		return a_player ?
		           m_conf.m_shield.m_flags.test(Flags::kPlayer) :
                   m_conf.m_shield.m_flags.test(Flags::kNPC);
	}

	bool Controller::GetShieldOnBackSwitch(Actor* a_actor) const
	{
		return a_actor != *g_thePlayer || m_shieldOnBackSwitch.load(std::memory_order_acquire) != 0;
	}

	bool Controller::GetShieldOnBackSwitch() const
	{
		return m_shieldOnBackSwitch.load(std::memory_order_acquire) != 0;
	}

	bool Controller::ShouldBlockShieldHide(Actor* a_actor) const
	{
		if (a_actor == *g_thePlayer)
		{
			if (!m_conf.m_shieldHideFlags.test(Flags::kPlayer))
			{
				return false;
			}
		}
		else
		{
			if (!m_conf.m_shieldHideFlags.test(Flags::kNPC))
			{
				return false;
			}
		}

		if (m_conf.m_shieldHideFlags.test(Flags::kMountOnly))
		{
			return a_actor->flags2.test(Actor::Flags2::kGettingOnOffMount) ||
			       a_actor->IsOnMount();
		}

		return true;
	}

	void Controller::ProcessEquippedShield(
		Actor*             a_actor,
		const NiRootNodes& a_roots,
		bool               a_drawn,
		bool               a_switch) const
	{
		if (!IsShieldEnabled(a_actor))
		{
			return;
		}

		auto bipedObject = GetShieldBipedObject(a_actor);
		if (bipedObject >= BIPED_OBJECT::kTotal)
		{
			return;
		}

		for (std::size_t i = 0; i < std::size(a_roots.m_nodes); i++)
		{
			auto& root = a_roots.m_nodes[i];

			if (!root)
			{
				continue;
			}

			bool firstPerson = (i == 1);

			if (firstPerson && !m_conf.m_shield.FirstPerson())
			{
				continue;
			}

			auto& bipedModel = a_actor->GetBiped1(firstPerson);
			if (!bipedModel)
			{
				continue;
			}

			auto& data = bipedModel->objects[stl::underlying(bipedObject)];

			auto form = data.item;
			if (!form)
			{
				continue;
			}

			auto armor = form->As<TESObjectARMO>();
			if (!armor)
			{
				continue;
			}

			if (!armor->IsShield())
			{
				continue;
			}

			auto& armorNode = data.object;
			if (!armorNode)
			{
				continue;
			}

			const auto& targetNodeName = (a_drawn || !a_switch) ?
			                                 m_strings->m_shield :
                                             m_strings->m_shieldSheathNode;

			auto targetNode = FindNode(root, targetNodeName);
			if (!targetNode)
			{
				continue;
			}

			AttachToNode(armorNode, targetNode);
		}
	}

	NiNode* Controller::GetScbAttachmentNode(
		Actor*         a_actor,
		TESObjectWEAP* a_weapon,
		NiNode*        a_root,
		bool           a_is1p) const
	{
		auto entry = m_data->Get(a_actor, a_weapon, true);
		if (!entry)
		{
			return nullptr;
		}

		if (a_is1p && !entry->FirstPerson())
		{
			return nullptr;
		}

		auto root = a_root;

		if (auto rootobj = root->GetObjectByName(m_strings->m_npcroot))
		{
			if (auto rootnode = rootobj->AsNode())
			{
				root = rootnode;
			}
		}

		return entry->GetNode(root, true);
	}

	const BSFixedString* Controller::GetScbAttachmentNodeName(NiNode*, TESObjectWEAP* a_form) const
	{
		return m_data->GetNodeName(a_form, true);
	}

	const BSFixedString* Controller::GetWeaponAttachmentNodeName(
		Actor*         a_actor,
		TESObjectWEAP* a_weapon,
		bool           a_is1p,
		bool           a_left) const
	{
		auto entry = m_data->Get(a_actor, a_weapon, a_left);
		if (!entry)
		{
			return nullptr;
		}

		if (a_is1p && !entry->FirstPerson())
		{
			return nullptr;
		}

		return std::addressof(entry->GetNodeName(a_left));
	}

	const BSFixedString* Controller::GetShieldAttachmentNodeName(
		Actor*         a_actor,
		TESObjectARMO* a_armor,
		bool           a_is1p) const
	{
		if (!a_armor->IsShield())
		{
			return nullptr;
		}

		if (!IsShieldEnabled(a_actor))
		{
			return nullptr;
		}

		if (!GetShieldOnBackSwitch(a_actor))
		{
			return nullptr;
		}

		if (a_actor->IsWeaponDrawn())
		{
			return nullptr;
		}

		if (a_is1p && !m_conf.m_shield.FirstPerson())
		{
			return nullptr;
		}

		return std::addressof(m_strings->m_shieldSheathNode);
	}

	void Controller::OnActorLoad(TESObjectREFR* a_actor) const
	{
		ITaskPool::QueueLoadedActorTask(a_actor, [this](Actor* a_actor, Game::ActorHandle) {
#ifdef _SDS_UNUSED
			m_nodeOverride->ApplyNodeOverrides(a_actor);
#endif
			ProcessWeaponDrawnChange(a_actor, a_actor->IsWeaponDrawn());

			if (m_conf.m_npcEquipLeft && ActorQualifiesForEquip(a_actor))
			{
				EvaluateEquip(a_actor);
			}
		});
	}

#ifdef _SDS_UNUSED

	void Controller::OnNiNodeUpdate(TESObjectREFR* a_actor)
	{
		ITaskPool::QueueActorTask(a_actor, [this](Actor* a_actor, Game::ActorHandle) {
			m_nodeOverride->ApplyNodeOverrides(a_actor);
		});
	}
#endif

	auto Controller::ReceiveEvent(
		const TESObjectLoadedEvent* a_evn,
		BSTEventSource<TESObjectLoadedEvent>*)
		-> EventResult
	{
		if (a_evn && a_evn->loaded)
		{
			if (auto actor = a_evn->formId.As<Actor>())
			{
				OnActorLoad(actor);
			}
		}

		return EventResult::kContinue;
	}

	auto Controller::ReceiveEvent(
		const TESInitScriptEvent* a_evn,
		BSTEventSource<TESInitScriptEvent>*)
		-> EventResult
	{
		if (a_evn)
		{
			OnActorLoad(a_evn->reference);
		}

		return EventResult::kContinue;
	}

	void Controller::OnWeaponEquip(Actor* a_actor, TESObjectWEAP* a_weapon)
	{
		if (!m_conf.m_npcEquipLeft)
		{
			return;
		}

		if (!ActorQualifiesForEquip(a_actor))
		{
			return;
		}

		auto pm = a_actor->processManager;
		if (!pm)
		{
			return;
		}

		if (a_weapon != pm->equippedObject[ActorProcessManager::kEquippedHand_Right])
		{
			return;
		}

		if (CanEquipEitherHand(a_weapon))
		{
			QueueEvaluateEquip(a_actor);
		}
	}

	auto Controller::ReceiveEvent(
		const TESEquipEvent* a_evn,
		BSTEventSource<TESEquipEvent>*)
		-> EventResult
	{
		if (a_evn && a_evn->equipped && a_evn->actor)
		{
			if (auto actor = a_evn->actor->As<Actor>(); actor)
			{
				if (auto weapon = a_evn->baseObject.As<TESObjectWEAP>(); weapon)
				{
					OnWeaponEquip(actor, weapon);
				}
			}
		}

		return EventResult::kContinue;
	}

	auto Controller::ReceiveEvent(
		const TESSwitchRaceCompleteEvent* a_evn,
		BSTEventSource<TESSwitchRaceCompleteEvent>*)
		-> EventResult
	{
		if (a_evn)
		{
			QueueProcessWeaponDrawnChange(
				a_evn->refr,
				DrawnState::Determine);
		}

		return EventResult::kContinue;
	}

	auto Controller::ReceiveEvent(
		const SKSENiNodeUpdateEvent* a_evn,
		BSTEventSource<SKSENiNodeUpdateEvent>*)
		-> EventResult
	{
		if (a_evn)
		{
			QueueProcessWeaponDrawnChange(
				a_evn->reference,
				DrawnState::Determine);

#ifdef _SDS_UNUSED
			OnNiNodeUpdate(a_evn->reference);
#endif
		}

		return EventResult::kContinue;
	}

	auto Controller::ReceiveEvent(
		const SKSEActionEvent* a_evn,
		BSTEventSource<SKSEActionEvent>*)
		-> EventResult
	{
		if (a_evn)
		{
			switch (a_evn->type)
			{
			case SKSEActionEvent::Type::kEndDraw:
				QueueProcessWeaponDrawnChange(a_evn->actor, DrawnState::Drawn);
				break;
			case SKSEActionEvent::Type::kEndSheathe:
				QueueProcessWeaponDrawnChange(a_evn->actor, DrawnState::Sheathed);
				break;
			}
		}

		return EventResult::kContinue;
	}

	void Controller::EvaluateDrawnStateOnNearbyActors()
	{
		ITaskPool::AddTask([this] {
			if (auto player = *g_thePlayer;
			    IsREFRValid(player))
			{
				ProcessWeaponDrawnChange(
					player,
					player->IsWeaponDrawn());
			}

			auto pl = Game::ProcessLists::GetSingleton();
			if (!pl)
			{
				return;
			}

			for (auto handle : pl->highActorHandles)
			{
				if (!handle || !handle.IsValid())
				{
					continue;
				}

				NiPointer<Actor> actor;

				if (!handle.Lookup(actor))
				{
					continue;
				}

				if (!IsREFRValid(actor))
				{
					continue;
				}

				ProcessWeaponDrawnChange(
					actor,
					actor->IsWeaponDrawn());
			}
		});
	}

	void Controller::Receive(const Events::OnSetEquipSlot&)
	{
		auto player = *g_thePlayer;
		if (!player || !player->loadedState)
		{
			return;
		}

		EvaluateDrawnStateOnNearbyActors();
	}

	void Controller::SaveGameHandler(SKSESerializationInterface* a_intfc)
	{
		a_intfc->OpenRecord('DSDS', stl::underlying(SerializationVersion::kDataVersion1));

		SerializedData data{
			m_shieldOnBackSwitch.load()
		};

		a_intfc->WriteRecordData(&data, sizeof(data));
	}

	void Controller::LoadGameHandler(SKSESerializationInterface* a_intfc)
	{
		std::uint32_t type, length, version;

		while (a_intfc->GetNextRecordInfo(&type, &version, &length))
		{
			switch (type)
			{
			case 'DSDS':
				{
					SerializedData data;

					if (a_intfc->ReadRecordData(std::addressof(data), sizeof(data)) == sizeof(data))
					{
						m_shieldOnBackSwitch.store(data.shieldOnBackSwitch);
					}
				}
				break;
			}
		}
	}

	void Controller::OnKeyPressed()
	{
		auto n = m_shieldOnBackSwitch.fetch_xor(1, std::memory_order_acq_rel);

		SDSPlayerShieldOnBackSwitchEvent evn{ !static_cast<bool>(n) };
		SendEvent(evn);

		ITaskPool::QueueLoadedActorTask(
			*g_thePlayer,
			[this](Actor* a_actor, Game::ActorHandle) {
				NiRootNodes roots(a_actor);
				roots.GetNPCRoots(m_strings->m_npcroot);

				bool drawn = a_actor->IsWeaponDrawn();
				bool sw    = GetShieldOnBackSwitch(a_actor);

				ProcessEquippedShield(a_actor, roots, drawn, sw);

				if (m_conf.m_shieldHandWorkaround &&
			        !drawn &&
			        IsShieldEquipped(a_actor))
				{
					std::int32_t value = sw ? 0 : 10;

					a_actor->SetVariableOnGraphsInt(
						m_strings->m_iLeftHandType,
						value);
				}
			});
	}

}
