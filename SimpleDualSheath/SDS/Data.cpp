#include "pch.h"

#include "Data.h"
#include "Config.h"
#include "Util/Node.h"

namespace SDS
{
    namespace Data
    {

        using namespace Util;

        Weapon::Weapon(
            const char* a_nodeName,
            const char* a_nodeNameLeft,
            const Config::ConfigEntry& a_config)
            :
            m_nodeName(a_nodeName),
            m_nodeNameLeft(a_config.m_sheathNode.empty() ? a_nodeNameLeft : a_config.m_sheathNode.c_str()),
            m_flags(a_config.m_flags)
        {
        }

        Weapon::Weapon(
            const Config::ConfigEntry& a_config)
            :
            m_flags(a_config.m_flags)
        {
        }

        const BSFixedString& Weapon::GetNodeName(bool a_left) const
        {
            bool swap = (m_flags & Flags::kSwap) == Flags::kSwap;

            if (a_left) {
                return swap ? m_nodeName : m_nodeNameLeft;
            }
            else {
                return swap ? m_nodeNameLeft : m_nodeName;
            }
        }

        NiNode* Weapon::GetNode(NiNode* a_root, bool a_left) const
        {
            return Node::FindNode(a_root, GetNodeName(a_left));
        }

        void WeaponData::SetStrings(
            std::uint32_t a_type,
            const char* a_nodeName,
            const char* a_nodeNameLeft)
        {
            if (a_type < std::size(m_entries))
            {
                auto entry = m_entries[a_type].get();
                if (entry)
                {
                    if (a_nodeName) {
                        entry->m_nodeName.Set(a_nodeName);
                    }

                    if (a_nodeNameLeft) {
                        entry->m_nodeNameLeft.Set(a_nodeNameLeft);
                    }
                }
            }
        }

        auto WeaponData::Get(
            Actor* a_actor,
            TESObjectWEAP* a_weapon,
            bool a_left) const
            -> const Weapon*
        {
            auto type = static_cast<std::uint32_t>(a_weapon->gameData.type);

            if (type < std::size(m_entries))
            {
                auto entry = m_entries[type].get();
                if (entry)
                {
                    if (a_actor == *g_thePlayer)
                    {
                        if ((entry->m_flags & Flags::kPlayer) != Flags::kPlayer) {
                            return nullptr;
                        }
                    }
                    else
                    {
                        if ((entry->m_flags & Flags::kNPC) != Flags::kNPC) {
                            return nullptr;
                        }
                    }

                    if (!a_left && (entry->m_flags & Flags::kRight) != Flags::kRight) {
                        return nullptr;
                    }

                    return entry;
                }
            }

            return nullptr;
        }

    }

}