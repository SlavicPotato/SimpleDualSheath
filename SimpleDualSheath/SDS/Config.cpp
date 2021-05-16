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
        {"NoFirstPerson", {Flags::kNoFirstPerson, false}},
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

    bool Config::Load(const std::string& a_path)
    {
        INIReader reader;

        reader.Load(a_path);
        bool ret = reader.ParseError() == 0;

        FlagParser flagParser;

        m_sword = flagParser.Parse(reader.Get("Sword", "Flags", "Player|NPC"));
        m_axe = flagParser.Parse(reader.Get("Axe", "Flags", "Player|NPC"));
        m_mace = flagParser.Parse(reader.Get("Mace", "Flags", "Player|NPC"));
        m_dagger = flagParser.Parse(reader.Get("Dagger", "Flags", "Player|NPC"));
        m_staff = flagParser.Parse(reader.Get("Staff", "Flags", "Player|NPC|Right"), true);

        m_scb = reader.Get("General", "ShowLeftWeaponHolster", true);

        return ret;
    }

}