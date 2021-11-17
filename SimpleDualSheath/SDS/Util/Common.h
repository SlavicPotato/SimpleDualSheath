#pragma once

namespace SDS
{
	namespace Util
	{
		namespace Common
		{
			bool IsREFRValid(const TESObjectREFR* a_refr);
			bool CanEquipEitherHand(TESObjectWEAP* item);
			bool IsShieldEquipped(Actor* a_actor);

		}
	}
}