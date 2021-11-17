#pragma once

namespace SDS
{
	namespace Events
	{
		struct CreateArmorNodeEvent
		{
			NiPointer<TESObjectREFR> reference;
			NiAVObject* object;
			Biped* info;
			BipedParam* params;
			NiNode* objectRoot;
		};
	}
}
