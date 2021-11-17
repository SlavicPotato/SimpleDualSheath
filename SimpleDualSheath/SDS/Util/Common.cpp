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
				if (a_refr == nullptr ||
				    a_refr->loadedState == nullptr ||
				    (a_refr->flags & TESForm::kFlagIsDeleted) == TESForm::kFlagIsDeleted)
				{
					return false;
				}
				return true;
			}

			bool CanEquipEitherHand(TESObjectWEAP* a_weapon)
			{
				auto equipSlot = a_weapon->equipType.GetEquipSlot();
				if (!equipSlot)
				{
					return false;
				}

				return (
					equipSlot == GetRightHandSlot() ||
					equipSlot == GetEitherHandSlot() ||
					equipSlot == GetLeftHandSlot());
			}

			bool IsShieldEquipped(Actor* a_actor)
			{
				if (!a_actor)
				{
					return false;
				}

				auto pm = a_actor->processManager;
				if (!pm)
				{
					return false;
				}

				auto form = pm->equippedObject[ActorProcessManager::kEquippedHand_Left];
				if (!form)
				{
					return false;
				}

				auto armor = form->As<TESObjectARMO>();
				if (!armor)
				{
					return false;
				}

				return armor->IsShield();
			}

		}
	}
}