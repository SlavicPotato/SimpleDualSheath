#pragma once

namespace SDS
{
	namespace Util
	{
		namespace Node
		{
			NiAVObject* GetNiObject(NiNode* a_root, const BSFixedString& a_name);

			void AttachToNode(
				NiPointer<NiAVObject>& a_object,
				NiPointer<NiNode>& a_node);

			void SetVisible(NiAVObject* a_object);

		}
	}
}