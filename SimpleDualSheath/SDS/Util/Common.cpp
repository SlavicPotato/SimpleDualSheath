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

            bool CanEquipEitherHand(TESForm* item)
            {
                auto equipType = RTTI<BGSEquipType>()(item);
                if (!equipType) {
                    return false;
                }

                auto equipSlot = equipType->GetEquipSlot();
                if (!equipSlot) {
                    return false;
                }

                return (
                    equipSlot == GetRightHandSlot() || 
                    equipSlot == GetEitherHandSlot() || 
                    equipSlot == GetLeftHandSlot());
            }

            bool IsShieldEquipped(Actor* a_actor)
            {
                if (!a_actor) {
                    return false;
                }

                auto pm = a_actor->processManager;
                if (!pm) {
                    return false;
                }

                auto form = pm->equippedObject[ActorProcessManager::kEquippedHand_Left];
                if (!form) {
                    return false;
                }

                if (form->formType != TESObjectARMO::kTypeID) {
                    return false;
                }

                auto armor = static_cast<TESObjectARMO*>(form);

                return armor->IsShield();
            }


        }
    }
}