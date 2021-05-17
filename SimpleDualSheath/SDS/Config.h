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
        Config(const std::string& a_path);

        bool Load(const std::string& a_path);
        SKMP_FORCEINLINE bool IsLoaded() const {
            return m_loaded;
        }

        Data::Flags m_sword;
        Data::Flags m_axe;
        Data::Flags m_mace;
        Data::Flags m_dagger;
        Data::Flags m_staff;
        bool m_scb;

        Data::Flags m_shield;
    private:

        bool m_loaded{ false };
    };
}