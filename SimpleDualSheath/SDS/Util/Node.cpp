#include "pch.h"

#include "Node.h"

namespace SDS
{
    namespace Util
    {
        namespace Node
        {
            NiAVObject* GetNiObject(
                NiNode* a_root,
                const BSFixedString& a_name)
            {
                return a_root->GetObjectByName(std::addressof(a_name.data));
            }

            NiNode* FindNode(
                NiNode* a_root,
                const BSFixedString& a_name)
            {
                auto object = GetNiObject(a_root, a_name);
                if (!object) {
                    return nullptr;
                }

                return object->GetAsNiNode();
            }

            void AttachToNode(NiAVObject* a_object, NiNode* a_node)
            {
                if (a_object->m_parent != a_node)
                {
                    if (a_object->m_parent) {
                        a_object->m_parent->RemoveChild(a_object);
                    }

                    a_node->AttachChild(a_object, true);
                }
            }
            
            void ClearCull(NiAVObject* a_object)
            {
                if ((a_object->m_flags & NiAVObject::kFlag_Cull) == NiAVObject::kFlag_Cull) {
                    a_object->m_flags &= ~NiAVObject::kFlag_Cull;
                }
            }

            NiRootNodes::NiRootNodes(
                TESObjectREFR* const a_ref,
                bool a_no1p)
                :
                m_nodes
            {
                a_ref->GetNiRootNode(false),
                a_no1p ? nullptr : a_ref->GetNiRootNode(true)
            }
            {
                if (m_nodes.firstPerson == m_nodes.thirdPerson) {
                    m_nodes.firstPerson = nullptr;
                }
            }

        }
    }
}