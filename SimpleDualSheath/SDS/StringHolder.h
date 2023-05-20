#pragma once

namespace SDS
{
	class StringHolder :
		public stl::intrusive_ref_counted
	{
	public:
		static inline constexpr auto NINODE_SWORD                = "WeaponSword";
		static inline constexpr auto NINODE_SWORD_LEFT           = "WeaponSwordLeft";
		static inline constexpr auto NINODE_SWORD_LEFT_SWP       = "WeaponSwordLeftSWP";
		static inline constexpr auto NINODE_AXE                  = "WeaponAxe";
		static inline constexpr auto NINODE_AXE_LEFT             = "WeaponAxeLeft";
		static inline constexpr auto NINODE_MACE                 = "WeaponMace";
		static inline constexpr auto NINODE_MACE_LEFT            = "WeaponMaceLeft";
		static inline constexpr auto NINODE_DAGGER               = "WeaponDagger";
		static inline constexpr auto NINODE_DAGGER_LEFT          = "WeaponDaggerLeft";
		static inline constexpr auto NINODE_STAFF                = "WeaponStaff";
		static inline constexpr auto NINODE_STAFF_LEFT           = "WeaponStaffLeft";
		static inline constexpr auto NINODE_SWORD_ON_BACK_LEFT   = "WeaponSwordLeftOnBack";
		static inline constexpr auto NINODE_AXE_ON_BACK_LEFT     = "WeaponAxeLeftOnBack";
		static inline constexpr auto NINODE_WEAPON_BACK          = "WeaponBack";
		static inline constexpr auto NINODE_WEAPON_BACK_SWP      = "WeaponBackSWP";
		static inline constexpr auto NINODE_WEAPON_BACK_AXE_MACE = "WeaponBackAxeMace";
		static inline constexpr auto NINODE_BOW                  = "WeaponBow";
		static inline constexpr auto NINODE_CROSSBOW             = "WeaponCrossbow";
		static inline constexpr auto NINODE_SHIELD_BACK          = "ShieldBack";
		static inline constexpr auto NINODE_SHIELD               = "SHIELD";
		static inline constexpr auto NINODE_WEAPON               = "WEAPON";
		static inline constexpr auto NINODE_NPCROOT              = "NPC Root [Root]";

		static inline constexpr auto iLeftHandType     = "iLeftHandType";
		static inline constexpr auto iLeftHandEquipped = "iLeftHandEquipped";

		static inline constexpr auto NINODE_SCB_LEFT = "ScbLeft";

		StringHolder();

		BSFixedString m_shieldSheathNode;

		BSFixedString m_shield;
		BSFixedString m_weapon;

		BSFixedString m_npcroot;

		BSFixedString m_iLeftHandType;
		BSFixedString m_iLeftHandEquipped;

		BSFixedString m_scbLeft;

		//BSFixedString m_weaponBack;
		BSFixedString m_weaponBackAxeMace;
		//BSFixedString m_weaponBow;
		BSFixedString m_weaponCrossbow;
	};

}