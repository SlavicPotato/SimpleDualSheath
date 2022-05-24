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

	class ConfigKeyCombo
	{
	public:
		ConfigKeyCombo() = default;
		void Parse(const std::string& a_input);

		[[nodiscard]] SKMP_FORCEINLINE bool Has() const
		{
			return m_key != 0;
		}

		[[nodiscard]] SKMP_FORCEINLINE auto GetKey() const
		{
			return m_key;
		}

		[[nodiscard]] SKMP_FORCEINLINE auto GetComboKey() const
		{
			return m_comboKey;
		}

	private:
		std::uint32_t m_key{ 0 };
		std::uint32_t m_comboKey{ 0 };
	};

	struct Config
	{
		inline static constexpr auto SECT_GENERAL = "General";
		inline static constexpr auto SECT_NPC     = "NPC";
		inline static constexpr auto SECT_SWORD   = "Sword";
		inline static constexpr auto SECT_AXE     = "Axe";
		inline static constexpr auto SECT_MACE    = "Mace";
		inline static constexpr auto SECT_DAGGER  = "Dagger";
		inline static constexpr auto SECT_STAFF   = "Staff";
		inline static constexpr auto SECT_SHIELD  = "ShieldOnBack";
		inline static constexpr auto SECT_2HSWORD = "2HSword";
		inline static constexpr auto SECT_2HAXE   = "2HAxe";

		inline static constexpr auto KW_FLAGS      = "Flags";
		inline static constexpr auto KW_SHEATHNODE = "SheathNode";

	public:
		struct ConfigEntry
		{
			stl::flag<Data::Flags> m_flags{ Data::Flags::kNone };
			std::string            m_sheathNode;

			[[nodiscard]] inline constexpr bool IsEnabled() const noexcept
			{
				return m_flags.test_any(Data::Flags::kEnabled);
			}

			[[nodiscard]] inline constexpr bool IsPlayerEnabled() const noexcept
			{
				return m_flags.test(Data::Flags::kPlayer);
			}

			[[nodiscard]] inline constexpr bool FirstPerson() const noexcept
			{
				return m_flags.test(Data::Flags::kFirstPerson);
			}
		};

		Config() = default;
		Config(const std::string& a_path);

		bool Load(const std::string& a_path);

		[[nodiscard]] inline constexpr bool IsLoaded() const noexcept
		{
			return m_loaded;
		}

		[[nodiscard]] inline constexpr bool HasEnabled2HEntries() const noexcept
		{
			return m_2hSword.IsEnabled() ||
			       m_2hAxe.IsEnabled();
		}

		ConfigEntry m_sword;
		ConfigEntry m_axe;
		ConfigEntry m_mace;
		ConfigEntry m_dagger;
		ConfigEntry m_staff;
		ConfigEntry m_2hSword;
		ConfigEntry m_2hAxe;
		ConfigEntry m_shield;

		bool m_scb{ true };
		bool m_scbCustom{ true };
		bool m_disableScabbards{ false };
		bool m_npcEquipLeft{ false };
		bool m_shieldHandWorkaround{ false };
		bool m_shwForceIfDrawn{ false };

		ConfigKeyCombo m_shieldToggleKeys;

		stl::flag<Data::Flags> m_shieldHideFlags{ Data::Flags::kNone };

	private:
		bool m_loaded{ false };
	};
}