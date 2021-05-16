#pragma once

#include "Data.h"

namespace SDS
{
    class FlagParser
    {
        using string_to_flag_map_t = stl::iunordered_map<std::string, std::pair<Data::Flags, bool>>;

    public:

        FlagParser();

        Data::Flags Parse(const std::string& a_in, bool a_internal = false);

    private:

        string_to_flag_map_t m_map;
    };

    struct Config
    {
    public:

        Config() = default;

        bool Load(const std::string& a_path);

        Data::Flags m_sword{ Data::Flags::kNone };
        Data::Flags m_axe{ Data::Flags::kNone };
        Data::Flags m_mace{ Data::Flags::kNone };
        Data::Flags m_dagger{ Data::Flags::kNone };
        Data::Flags m_staff{ Data::Flags::kNone };

        bool m_scb{ true };
    };
}