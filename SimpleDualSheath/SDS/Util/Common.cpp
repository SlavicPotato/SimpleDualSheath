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
                if (!equipType)
                    return false;

                auto equipSlot = equipType->GetEquipSlot();
                if (!equipSlot)
                    return false;

                return (equipSlot == GetRightHandSlot() || equipSlot == GetEitherHandSlot() || equipSlot == GetLeftHandSlot());
            }

        }
    }
}