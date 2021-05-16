#pragma once

namespace SDS
{
    class StringHolder
    {
    public:

        static inline constexpr auto NINODE_SWORD = "WeaponSword";
        static inline constexpr auto NINODE_SWORD_LEFT = "WeaponSwordLeft";

        static inline constexpr auto NINODE_AXE = "WeaponAxe";
        static inline constexpr auto NINODE_AXE_LEFT = "WeaponAxeLeft";

        static inline constexpr auto NINODE_MACE = "WeaponMace";
        static inline constexpr auto NINODE_MACE_LEFT = "WeaponMaceLeft";

        static inline constexpr auto NINODE_DAGGER = "WeaponDagger";
        static inline constexpr auto NINODE_DAGGER_LEFT = "WeaponDaggerLeft";

        static inline constexpr auto NINODE_STAFF = "WeaponStaff";
        static inline constexpr auto NINODE_STAFF_LEFT = "WeaponStaffLeft";

        static inline constexpr auto NINODE_SHIELD = "SHIELD";
        static inline constexpr auto NINODE_WEAPON = "WEAPON";

        StringHolder();

        BSFixedString m_shield;
        BSFixedString m_weapon;
    };

}