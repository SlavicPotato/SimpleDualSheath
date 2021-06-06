#pragma once

namespace SDS
{
    class StringHolder
    {
    public:

        static inline constexpr auto NINODE_SWORD = "WeaponSword";
        static inline constexpr auto NINODE_SWORD_LEFT = "WeaponSwordLeft";
        static inline constexpr auto NINODE_SWORD_LEFT_SWP = "WeaponSwordLeftSWP";

        static inline constexpr auto NINODE_AXE = "WeaponAxe";
        static inline constexpr auto NINODE_AXE_LEFT = "WeaponAxeLeft";

        static inline constexpr auto NINODE_MACE = "WeaponMace";
        static inline constexpr auto NINODE_MACE_LEFT = "WeaponMaceLeft";

        static inline constexpr auto NINODE_DAGGER = "WeaponDagger";
        static inline constexpr auto NINODE_DAGGER_LEFT = "WeaponDaggerLeft";

        static inline constexpr auto NINODE_STAFF = "WeaponStaff";
        static inline constexpr auto NINODE_STAFF_LEFT = "WeaponStaffLeft";

        static inline constexpr auto NINODE_SWORD_ON_BACK_LEFT = "WeaponSwordLeftOnBack";
        static inline constexpr auto NINODE_AXE_ON_BACK_LEFT = "WeaponAxeLeftOnBack";

        static inline constexpr auto NINODE_WEAPON_BACK = "WeaponBack";
        static inline constexpr auto NINODE_WEAPON_BACK_SWP = "WeaponBackSWP";
        static inline constexpr auto NINODE_SHIELD_BACK = "ShieldBack";

        static inline constexpr auto NINODE_SHIELD = "SHIELD";
        static inline constexpr auto NINODE_WEAPON = "WEAPON";

        static inline constexpr auto NINODE_NPCROOT = "NPC Root [Root]";

        static inline constexpr auto iLeftHandType = "iLeftHandType";

        StringHolder();

        BSFixedString m_shieldSheathNode;

        BSFixedString m_shield;
        BSFixedString m_weapon;

        BSFixedString m_npcroot;

        BSFixedString m_iLeftHandType;

#ifdef _SDS_UNUSED

        static inline constexpr auto NINODE_SWORD_BACK_MOV = "MOV WeaponSwordOnBack";
        static inline constexpr auto NINODE_SWORD_LEFT_BACK_MOV = "MOV WeaponSwordLeftOnBack";
        static inline constexpr auto NINODE_SWORD_SWP_MOV = "MOV WeaponSwordSWP";
        static inline constexpr auto NINODE_SWORD_LEFT_SWP_MOV = "MOV WeaponSwordLeftSWP";

        static inline constexpr auto NINODE_SWORD_DEFAULT_MOV = "MOV WeaponSwordDefault";
        static inline constexpr auto NINODE_SWORD_LEFT_DEFAULT_MOV = "MOV WeaponSwordLeftDefault";

        static inline constexpr auto NINODE_AXE_BACK_MOV = "MOV WeaponAxeOnBack";
        static inline constexpr auto NINODE_AXE_BACK_LEFT_MOV = "MOV WeaponAxeLeftOnBack";
        
        static inline constexpr auto NINODE_AXE_DEFAULT_MOV = "MOV WeaponAxeDefault";
        static inline constexpr auto NINODE_AXE_LEFT_DEFAULT_MOV = "MOV WeaponAxeLeftDefault";
        
        static inline constexpr auto NINODE_DAGGER_BACK_HIP_MOV = "MOV WeaponDaggerBackHip";
        static inline constexpr auto NINODE_DAGGER_BACK_HIP_LEFT_MOV = "MOV WeaponDaggerLeftBackHip";
        
        static inline constexpr auto NINODE_DAGGER_ANKLE_MOV = "MOV WeaponDaggerAnkle";
        static inline constexpr auto NINODE_DAGGER_ANKLE_LEFT_MOV = "MOV WeaponDaggerLeftAnkle";
        
        static inline constexpr auto NINODE_DAGGER_DEFAULT_MOV = "MOV WeaponDaggerDefault";
        static inline constexpr auto NINODE_DAGGER_LEFT_DEFAULT_MOV = "MOV WeaponDaggerLeftDefault";

        BSFixedString m_weaponSword;
        BSFixedString m_weaponSwordLeft;
        BSFixedString m_weaponSwordOnBackMOV;
        BSFixedString m_weaponSwordLeftOnBackMOV;
        BSFixedString m_weaponSwordSWPMOV;
        BSFixedString m_weaponSwordLeftSWPMOV;

        BSFixedString m_weaponSwordDefaultMOV;
        BSFixedString m_weaponSwordLeftDefaultMOV;

        BSFixedString m_weaponAxe;
        BSFixedString m_weaponAxeLeft;
        BSFixedString m_weaponAxeOnBackMOV;
        BSFixedString m_weaponAxeLeftOnBackMOV;

        BSFixedString m_weaponAxeDefaultMOV;
        BSFixedString m_weaponAxeLeftDefaultMOV;

        BSFixedString m_weaponDagger;
        BSFixedString m_weaponDaggerLeft;
        BSFixedString m_weaponDaggerBackHipMOV;
        BSFixedString m_weaponDaggerLeftBackHipMOV;
        BSFixedString m_weaponDaggerAnkleMOV;
        BSFixedString m_weaponDaggerLeftAnkleMOV;
        BSFixedString m_weaponDaggerDefaultMOV;
        BSFixedString m_weaponDaggerLeftDefaultMOV;

#endif
    };

}