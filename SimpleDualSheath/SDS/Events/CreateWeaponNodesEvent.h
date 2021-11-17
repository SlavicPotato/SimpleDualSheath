#pragma once

namespace SDS
{
	namespace Events
	{
		struct CreateWeaponNodesEvent
		{
			TESObjectREFR* reference;
			TESForm* object;
			bool left;
		};
	}
}