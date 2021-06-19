#pragma once

namespace SDS
{
    namespace Util
    {
        namespace Common
        {
            bool IsREFRValid(const TESObjectREFR* a_refr);
            bool CanEquipEitherHand(TESForm* item);
            bool IsShieldEquipped(Actor* a_actor);
            TESRace* GetActorRace(Actor* a_actor);

        }
    }
}