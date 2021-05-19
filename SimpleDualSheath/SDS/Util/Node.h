#pragma once

namespace SDS
{
    namespace Util
    {
        namespace Node
        {
            NiAVObject* GetNiObject(NiNode* a_root, const BSFixedString& a_name);

            [[nodiscard]] NiNode* FindNode(NiNode* a_root, const BSFixedString& a_name);
            void AttachToNode(NiPointer<NiAVObject>& a_object, NiPointer<NiNode> &a_node, bool a_update = false);
            void ClearCull(NiAVObject* a_object);

            struct NiRootNodes
            {
                NiRootNodes(TESObjectREFR* const a_ref, bool a_no1p = false);

                [[nodiscard]] SKMP_FORCEINLINE bool MatchesAny(
                    NiNode* const a_node)
                {
                    return a_node == m_nodes[0] || a_node == m_nodes[1];
                }

                void GetNPCRoots(const BSFixedString &a_npcroot);

                NiPointer<NiNode> m_nodes[2];
            };

        }
    }
}