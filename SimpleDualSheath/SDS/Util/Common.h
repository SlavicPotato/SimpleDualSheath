#pragma once

namespace SDS
{
	namespace Util
	{
		namespace Common
		{
			bool IsREFRValid(const TESObjectREFR* a_refr);
			bool CanEquipEitherHand(const TESObjectWEAP* item);
			bool IsShieldEquipped(const Actor* a_actor);

		}
	}
}