#include "pch.h"

#include "Config.h"
#include "StringHolder.h"

namespace SDS
{
	using namespace Data;

	FlagParser::FlagParser() :
		m_map{
			{ "NPC", { Flags::kNPC, false } },
			{ "Player", { Flags::kPlayer, false } },
			{ "FirstPerson", { Flags::kFirstPerson, false } },
			{ "MountOnly", { Flags::kMountOnly, false } },
			{ "Right", { Flags::kRight, true } },
			{ "Swap", { Flags::kSwap, true } }
		}
	{
	}

	auto FlagParser::Parse(
		const std::string& a_in,
		bool               a_internal)
		-> Flags
	{
		std::vector<std::string> v;
		StrHelpers::SplitString(a_in, '|', v, true);

		Flags out(Flags::kNone);

		for (const auto& e : v)
		{
			auto it = m_map.find(e);
			if (it != m_map.end())
			{
				if (it->second.second && !a_internal)
				{
					continue;
				}

				out |= it->second.first;
			}
		}

		return out;
	}

	void ConfigKeyCombo::Parse(
		const std::string& a_input)
	{
		std::vector<std::uint32_t> e;
		StrHelpers::SplitString(a_input, '+', e, true, true);

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

		FlagParser flagParser;

		m_scb              = reader.GetBoolValue(SECT_GENERAL, "EnableLeftScabbards", true);
		m_scbCustom        = reader.GetBoolValue(SECT_GENERAL, "CustomLeftScabbards", true);
		m_disableScabbards = reader.GetBoolValue(SECT_GENERAL, "DisableAllScabbards", false);

		m_sword = {
			flagParser.Parse(reader.GetValue(SECT_SWORD, KW_FLAGS, "Player|NPC")),
			reader.GetValue(SECT_SWORD, KW_SHEATHNODE, StringHolder::NINODE_SWORD_LEFT)
		};

		m_axe = {
			flagParser.Parse(reader.GetValue(SECT_AXE, KW_FLAGS, "Player|NPC")),
			reader.GetValue(SECT_AXE, KW_SHEATHNODE, StringHolder::NINODE_AXE_LEFT)
		};

		m_mace = {
			flagParser.Parse(reader.GetValue(SECT_MACE, KW_FLAGS, "Player|NPC")),
			reader.GetValue(SECT_MACE, KW_SHEATHNODE, StringHolder::NINODE_MACE_LEFT)
		};

		m_dagger = {
			flagParser.Parse(reader.GetValue(SECT_DAGGER, KW_FLAGS, "Player|NPC")),
			reader.GetValue(SECT_DAGGER, KW_SHEATHNODE, StringHolder::NINODE_DAGGER_LEFT)
		};

		m_2hSword = {
			flagParser.Parse(reader.GetValue(SECT_2HSWORD, KW_FLAGS, "")),
			reader.GetValue(SECT_2HSWORD, KW_SHEATHNODE, StringHolder::NINODE_SWORD_ON_BACK_LEFT)
		};

		m_2hAxe = {
			flagParser.Parse(reader.GetValue(SECT_2HAXE, KW_FLAGS, "")),
			reader.GetValue(SECT_2HAXE, KW_SHEATHNODE, StringHolder::NINODE_AXE_ON_BACK_LEFT)
		};

		m_staff = {
			flagParser.Parse(reader.GetValue(SECT_STAFF, KW_FLAGS, "Player|NPC|Right"), true),
			reader.GetValue(SECT_STAFF, KW_SHEATHNODE, StringHolder::NINODE_STAFF_LEFT)
		};

		m_shield = {
			flagParser.Parse(reader.GetValue(SECT_SHIELD, KW_FLAGS, "")),
			reader.GetValue(SECT_SHIELD, KW_SHEATHNODE, StringHolder::NINODE_SHIELD_BACK)
		};

		m_shieldHandWorkaround = reader.GetBoolValue(SECT_SHIELD, "ClenchedHandWorkaround", false);
		m_shwForceIfDrawn      = reader.GetBoolValue(SECT_SHIELD, "ClenchedHandWorkaroundForceIfDrawn", false);
		m_shieldHideFlags      = flagParser.Parse(reader.GetValue(SECT_SHIELD, "DisableHideOnSit", ""));
		m_shieldToggleKeys.Parse(reader.GetValue(SECT_SHIELD, "ToggleKeys", ""));

		m_npcEquipLeft = reader.GetBoolValue(SECT_NPC, "EquipLeft", false);

		return (m_loaded = reader.is_loaded());
	}

}