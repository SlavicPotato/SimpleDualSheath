#pragma once

#include "Flags.h"

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
        inline static constexpr auto SECT_SWORD = "Sword";
        inline static constexpr auto SECT_GENERAL = "General";
        inline static constexpr auto SECT_AXE = "Axe";
        inline static constexpr auto SECT_MACE = "Mace";
        inline static constexpr auto SECT_DAGGER = "Dagger";
        inline static constexpr auto SECT_STAFF = "Staff";
        inline static constexpr auto SECT_SHIELD = "ShieldOnBack";

        inline static constexpr auto KW_FLAGS = "Flags";
        inline static constexpr auto KW_SHEATHNODE = "SheathNode";

    public:

        struct ConfigEntry
        {
            Data::Flags m_flags;
            std::string m_sheathNode;
        };

        Config() = default;
        Config(const std::string& a_path);

        bool Load(const std::string& a_path);
        SKMP_FORCEINLINE bool IsLoaded() const {
            return m_loaded;
        }

        ConfigEntry m_sword;
        ConfigEntry m_axe;
        ConfigEntry m_mace;
        ConfigEntry m_dagger;
        ConfigEntry m_staff;
        ConfigEntry m_shield;

        bool m_scb;

    private:

        bool m_loaded{ false };
    };
}