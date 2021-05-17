#pragma once

namespace SDS
{
    namespace Util
    {
        namespace Node
        {
            NiAVObject* GetNiObject(NiNode* a_root, const BSFixedString& a_name);

            [[nodiscard]] NiNode* FindNode(NiNode* a_root, const BSFixedString& a_name);
            void AttachToNode(NiAVObject* a_object, NiNode* a_node);
            void ClearCull(NiAVObject* a_object);

            struct NiRootNodes
            {
                NiRootNodes(TESObjectREFR* const a_ref, bool a_no1p = false);

                [[nodiscard]] SKMP_FORCEINLINE bool MatchesAny(
                    NiNode* const a_node)
                {
                    return a_node == m_nodes.thirdPerson || a_node == m_nodes.firstPerson;
                }

                union
                {
                    struct
                    {
                        NiNode* thirdPerson;
                        NiNode* firstPerson;
                    } m_nodes;

                    NiNode* m_arr[2];

                    static_assert(sizeof(m_arr) == sizeof(m_nodes));
                };
            };

        }
    }
}