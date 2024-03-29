#pragma once

#include "Config.h"
#include "Flags.h"

namespace SDS
{
	namespace Data
	{
		class Weapon
		{
		public:
			Weapon(
				const char*                     a_nodeName,
				const char*                     a_nodeNameLeft,
				const SDS::Config::ConfigEntry& a_config);

			[[nodiscard]] const BSFixedString& GetNodeName(bool a_left) const;
			[[nodiscard]] NiNode*              GetNode(NiNode* a_root, bool a_left) const;

			[[nodiscard]] inline constexpr bool FirstPerson() const noexcept
			{
				return m_flags.test(Flags::kFirstPerson);
			}

			BSFixedString    m_nodeName;
			BSFixedString    m_nodeNameLeft;
			stl::flag<Flags> m_flags{ Flags::kNone };
		};

		class WeaponData
		{
		public:
			WeaponData() = default;

			template <class... Args>
			void Create(WEAPON_TYPE a_type, Args&&... a_args)
			{
				if (stl::underlying(a_type) < std::size(m_entries))
				{
					m_entries[stl::underlying(a_type)] = std::make_unique<Weapon>(std::forward<Args>(a_args)...);
				}
			}

			void SetStrings(std::uint32_t a_type, const char* a_nodeName, const char* a_nodeNameLeft);

			[[nodiscard]] const Weapon*        Get(Actor* a_actor, const TESObjectWEAP* a_weapon, bool a_left) const;
			[[nodiscard]] const BSFixedString* GetNodeName(const TESObjectWEAP* a_weapon, bool a_left) const;

		private:
			std::unique_ptr<Weapon> m_entries[10];
		};

	}
}