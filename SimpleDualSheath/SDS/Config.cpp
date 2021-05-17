#include "pch.h"

#include "Config.h"

namespace SDS
{
    using namespace Data;

    FlagParser::FlagParser() :
        m_map
    {
        {"NPC", {Flags::kNPC, false}},
        {"Player", {Flags::kPlayer, false}},
        {"FirstPerson", {Flags::kFirstPerson, false}},
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

        m_scb = reader.Get("General", "ShowLeftWeaponHolster", true);

        m_sword = flagParser.Parse(reader.Get("Sword", "Flags", "Player|NPC"));
        m_axe = flagParser.Parse(reader.Get("Axe", "Flags", "Player|NPC"));
        m_mace = flagParser.Parse(reader.Get("Mace", "Flags", "Player|NPC"));
        m_dagger = flagParser.Parse(reader.Get("Dagger", "Flags", "Player|NPC"));
        m_staff = flagParser.Parse(reader.Get("Staff", "Flags", "Player|NPC|Right"), true);

        m_shield = flagParser.Parse(reader.Get("Shield", "Flags", ""));

        return (m_loaded = (reader.ParseError() == 0));
    }

}