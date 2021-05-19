#include "pch.h"

#include "Config.h"
#include "StringHolder.h"

namespace SDS
{
    using namespace Data;

    FlagParser::FlagParser() :
        m_map
    {
        {"NPC", {Flags::kNPC, false}},
        {"Player", {Flags::kPlayer, false}},
        {"FirstPerson", {Flags::kFirstPerson, false}},
        {"Immediate", {Flags::kImmediate, false}},
        {"UpdateNode", {Flags::kUpdateNodeOnAttach, false}},
        {"Right", {Flags::kRight, true}},
        {"Swap", {Flags::kSwap, true}}
    }
    {
    }

    auto FlagParser::Parse(
        const std::string& a_in,
        bool a_internal)
        -> Flags
    {
        stl::vector<std::string> v;
        StrHelpers::SplitString(a_in, '|', v, true);

        Flags out(Flags::kNone);

        for (const auto& e : v)
        {
            auto it = m_map.find(e);
            if (it != m_map.end())
            {
                if (it->second.second && !a_internal) {
                    continue;
                }

                out |= it->second.first;
            }
        }

        return out;

    }

    Config::Config(const std::string& a_path)
    {
        Load(a_path);
    }

    bool Config::Load(const std::string& a_path)
    {
        INIReader reader(a_path);

        FlagParser flagParser;

        m_scb = reader.Get(SECT_GENERAL, "EnableLeftScabbards", true);

        m_sword = {
            flagParser.Parse(reader.Get(SECT_SWORD, KW_FLAGS, "Player|NPC")),
            reader.Get(SECT_SWORD, KW_SHEATHNODE, StringHolder::NINODE_SWORD_LEFT)
        };

        m_axe = {
            flagParser.Parse(reader.Get(SECT_AXE, KW_FLAGS, "Player|NPC")),
            reader.Get(SECT_AXE, KW_SHEATHNODE, StringHolder::NINODE_AXE_LEFT)
        };

        m_mace = {
            flagParser.Parse(reader.Get(SECT_MACE, KW_FLAGS, "Player|NPC")),
            reader.Get(SECT_MACE, KW_SHEATHNODE, StringHolder::NINODE_MACE_LEFT)
        };

        m_dagger = {
            flagParser.Parse(reader.Get(SECT_DAGGER, KW_FLAGS, "Player|NPC")),
            reader.Get(SECT_DAGGER, KW_SHEATHNODE, StringHolder::NINODE_DAGGER_LEFT)
        };

        m_staff = {
            flagParser.Parse(reader.Get(SECT_STAFF, KW_FLAGS, "Player|NPC|Right"), true),
            reader.Get(SECT_STAFF, KW_SHEATHNODE, StringHolder::NINODE_STAFF_LEFT)
        };

        m_shield = {
            flagParser.Parse(reader.Get(SECT_SHIELD, KW_FLAGS, "")),
            reader.Get(SECT_SHIELD, KW_SHEATHNODE, StringHolder::NINODE_SHIELD_BACK)
        };

        return (m_loaded = (reader.ParseError() == 0));
    }

}