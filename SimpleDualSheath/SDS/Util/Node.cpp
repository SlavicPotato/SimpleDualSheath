#include "pch.h"

#include "Node.h"

namespace SDS
{
	namespace Util
	{
		namespace Node
		{
			NiAVObject* GetNiObject(
				NiNode*              a_root,
				const BSFixedString& a_name)
			{
				return a_root->GetObjectByName(a_name);
			}

			void AttachToNode(
				NiAVObject* a_object,
				NiNode*     a_node)
			{
				if (a_object->m_parent != a_node)
				{
					a_node->AttachChild(a_object, true);
				}
			}

		}
	}
}