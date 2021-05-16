#include "pch.h"

#include "Node.h"

namespace SDS
{
    namespace Util
    {
        namespace Node
        {
            void GetNiObject(
                const BSFixedString& a_name,
                NiNode* a_root,
                NiPointer<NiAVObject>& a_result)
            {
                a_result = a_root->GetObjectByName(std::addressof(a_name.data));
            }

            NiNode* FindNode(
                NiNode* a_root,
                const BSFixedString& a_name)
            {
                auto object = a_root->GetObjectByName(std::addressof(a_name.data));
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

            bool NiRootNodes::MatchesAny(
                NiNode* const a_node)
            {
                return a_node == m_nodes.thirdPerson || a_node == m_nodes.firstPerson;
            }

        }
    }
}