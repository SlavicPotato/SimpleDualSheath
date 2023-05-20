#include "pch.h"

#include "Common.h"

namespace SDS
{
	namespace Util
	{
		namespace Common
		{
			bool IsREFRValid(const TESObjectREFR* a_refr)
			{
				if (!a_refr ||
				    a_refr->formID == 0 ||
				    !a_refr->loadedState ||
				    a_refr->IsDeleted())
				{
					return false;
				}
				return true;
			}

			bool CanEquipEitherHand(const TESObjectWEAP* a_weapon)
			{
				auto equipSlot = a_weapon->GetEquipSlot();
				if (!equipSlot)
				{
					return false;
				}

				return (
					equipSlot == GetRightHandSlot() ||
					equipSlot == GetEitherHandSlot() ||
					equipSlot == GetLeftHandSlot());
			}

			bool IsShieldEquipped(const Actor* a_actor)
			{
				if (!a_actor)
				{
					return false;
				}

				const auto* const pm = a_actor->processManager;
				if (!pm)
				{
					return false;
				}

				const auto* const form = pm->equippedObject[ActorProcessManager::kEquippedHand_Left];
				if (!form)
				{
					return false;
				}

				const auto armor = form->As<const TESObjectARMO>();
				if (!armor)
				{
					return false;
				}

				return armor->IsShield();
			}

		}
	}
}