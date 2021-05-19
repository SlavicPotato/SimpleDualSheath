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

            void AttachToNode(NiPointer<NiAVObject> &a_object, NiPointer<NiNode>& a_node, bool a_update)
            {
                if (NiPointer parent = a_object->m_parent; parent != a_node)
                {
                    if (parent) {
                        parent->RemoveChild(a_object);
                    }

                    a_node->AttachChild(a_object, true);

                    if (a_update)
                    {
                        NiAVObject::ControllerUpdateContext ctx{ 0.0f, 0 };
                        a_object->UpdateNode(std::addressof(ctx));
                    }
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
            {
                auto root3p = a_ref->GetNiRootNode(false);
                auto root1p = a_no1p ? nullptr : a_ref->GetNiRootNode(true);

                m_nodes[0] = root3p;

                if (root3p == root1p) {
                    m_nodes[1] = nullptr;
                }
                else {
                    m_nodes[1] = root1p;
                }
            }

            void NiRootNodes::GetNPCRoots(const BSFixedString& a_npcroot)
            {
                for (std::size_t i = 0; i < std::size(m_nodes); i++)
                {
                    auto &root = m_nodes[i];

                    if (!root) {
                        continue;
                    }

                    auto n = FindNode(root, a_npcroot);
                    if (n) {
                        m_nodes[i] = n;
                    }
                }
            }

        }
    }
}