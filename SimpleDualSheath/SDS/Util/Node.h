#pragma once

namespace SDS
{
	namespace Util
	{
		namespace Node
		{
			NiAVObject* GetNiObject(NiNode* a_root, const BSFixedString& a_name);

			void AttachToNode(
				NiAVObject* a_object,
				NiNode*     a_node);

			void SetVisible(NiAVObject* a_object);

		}
	}
}