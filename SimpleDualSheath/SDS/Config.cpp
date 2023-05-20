#include "pch.h"

#include "Config.h"
#include "StringHolder.h"

namespace SDS
{
	using namespace Data;

	using flag_key_hasher =
		stl::hasher<
			stl::fnv_hasher<std::uint64_t, stl::fnv_variant::fnv1a>,
			stl::charproc_tolower>;

	static_assert(flag_key_hasher::hash_string("NPC") == flag_key_hasher::hash_string("nPc"));

	constexpr auto s_flag_data = stl::make_array(

		std::make_tuple(flag_key_hasher::hash_string("NPC"), Flags::kNPC, false),
		std::make_tuple(flag_key_hasher::hash_string("Player"), Flags::kPlayer, false),
		std::make_tuple(flag_key_hasher::hash_string("FirstPerson"), Flags::kFirstPerson, false),
		std::make_tuple(flag_key_hasher::hash_string("MountOnly"), Flags::kMountOnly, false),
		std::make_tuple(flag_key_hasher::hash_string("Right"), Flags::kRight, true),
		std::make_tuple(flag_key_hasher::hash_string("Swap"), Flags::kSwap, true)

	);

	auto FlagParser::Parse(
		const std::string& a_in,
		bool               a_internal)
		-> Flags
	{
		std::vector<std::string> v;
		stl::split_string(a_in, '|', v, true);

		auto out = Flags::kNone;

		for (const auto& e : v)
		{
			const auto h = flag_key_hasher::hash_string(e);

			const auto it = std::find_if(
				s_flag_data.begin(),
				s_flag_data.end(),
				[&](auto& a_v) {
					return std::get<0>(a_v) == h;
				});

			if (it != s_flag_data.end())
			{
				if (!std::get<2>(*it) || a_internal)
				{
					out |= std::get<1>(*it);
				}
			}
		}

		return out;
	}

	void ConfigKeyCombo::Parse(
		const std::string& a_input)
	{
		std::vector<std::uint32_t> e;
		stl::split_string(a_input, '+', e, true, true);

		m_comboKey = 0;
		m_key      = 0;

		auto n = e.size();

		if (n > 1)
		{
			m_comboKey = e[0];
			m_key      = e[1];
		}
		else if (n == 1)
		{
			m_key = e[0];
		}
	}

	Config::Config(const std::string& a_path)
	{
		Load(a_path);
	}

	bool Config::Load(const std::string& a_path)
	{
		INIConfReader reader(a_path);

		m_disableScabbards       = reader.GetBoolValue(SECT_GENERAL, "DisableAllScabbards", false);
		m_disableWeapNodeSharing = reader.GetBoolValue(SECT_GENERAL, "DisableWeaponNodeSharing", false);

		m_sword = {
			FlagParser::Parse(reader.GetValue(SECT_SWORD, KW_FLAGS, "Player|NPC")),
			reader.GetValue(SECT_SWORD, KW_SHEATHNODE, StringHolder::NINODE_SWORD_LEFT)
		};

		m_axe = {
			FlagParser::Parse(reader.GetValue(SECT_AXE, KW_FLAGS, "Player|NPC")),
			reader.GetValue(SECT_AXE, KW_SHEATHNODE, StringHolder::NINODE_AXE_LEFT)
		};

		m_mace = {
			FlagParser::Parse(reader.GetValue(SECT_MACE, KW_FLAGS, "Player|NPC")),
			reader.GetValue(SECT_MACE, KW_SHEATHNODE, StringHolder::NINODE_MACE_LEFT)
		};

		m_dagger = {
			FlagParser::Parse(reader.GetValue(SECT_DAGGER, KW_FLAGS, "Player|NPC")),
			reader.GetValue(SECT_DAGGER, KW_SHEATHNODE, StringHolder::NINODE_DAGGER_LEFT)
		};

		m_2hSword = {
			FlagParser::Parse(reader.GetValue(SECT_2HSWORD, KW_FLAGS, "")),
			reader.GetValue(SECT_2HSWORD, KW_SHEATHNODE, StringHolder::NINODE_SWORD_ON_BACK_LEFT)
		};

		m_2hAxe = {
			FlagParser::Parse(reader.GetValue(SECT_2HAXE, KW_FLAGS, "")),
			reader.GetValue(SECT_2HAXE, KW_SHEATHNODE, StringHolder::NINODE_AXE_ON_BACK_LEFT)
		};

		m_staff = {
			FlagParser::Parse(reader.GetValue(SECT_STAFF, KW_FLAGS, "Player|NPC|Right"), true),
			reader.GetValue(SECT_STAFF, KW_SHEATHNODE, StringHolder::NINODE_STAFF_LEFT)
		};

		m_shield = {
			FlagParser::Parse(reader.GetValue(SECT_SHIELD, KW_FLAGS, "")),
			reader.GetValue(SECT_SHIELD, KW_SHEATHNODE, StringHolder::NINODE_SHIELD_BACK)
		};

		m_shieldHandWorkaround = reader.GetBoolValue(SECT_SHIELD, "ClenchedHandWorkaround", false);
		m_shwForceIfDrawn      = reader.GetBoolValue(SECT_SHIELD, "ClenchedHandWorkaroundForceIfDrawn", false);
		m_shieldHideFlags      = FlagParser::Parse(reader.GetValue(SECT_SHIELD, "DisableHideOnSit", ""));
		m_shieldToggleKeys.Parse(reader.GetValue(SECT_SHIELD, "ToggleKeys", ""));

		m_npcEquipLeft = reader.GetBoolValue(SECT_NPC, "EquipLeft", false);

		return (m_loaded = reader.is_loaded());
	}

}