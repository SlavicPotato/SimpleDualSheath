#pragma once

namespace SDS
{
    namespace Util
    {
        namespace Node
        {
            void GetNiObject(
                const BSFixedString& a_name,
                NiNode* a_root,
                NiPointer<NiAVObject>& a_result);

            [[nodiscard]] NiNode* FindNode(NiNode* a_root, const BSFixedString& a_name);
            void AttachToNode(NiAVObject* a_object, NiNode* a_node);

            struct NiRootNodes
            {
                NiRootNodes(TESObjectREFR* const a_ref, bool a_no1p = false);

                [[nodiscard]] bool MatchesAny(NiNode* const a_node);

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