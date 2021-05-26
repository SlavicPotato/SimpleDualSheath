#include "pch.h"

#include "StringHolder.h"

namespace SDS
{
    StringHolder::StringHolder() :
        m_shield(NINODE_SHIELD),
        m_weapon(NINODE_WEAPON),
        m_npcroot(NINODE_NPCROOT),
        m_shieldSheathNode(NINODE_SHIELD_BACK)
#ifdef _SDS_UNUSED
        ,
        m_weaponSword(NINODE_SWORD),
        m_weaponSwordLeft(NINODE_SWORD_LEFT),
        m_weaponSwordOnBackMOV(NINODE_SWORD_BACK_MOV),
        m_weaponSwordLeftOnBackMOV(NINODE_SWORD_LEFT_BACK_MOV),
        m_weaponSwordSWPMOV(NINODE_SWORD_SWP_MOV),
        m_weaponSwordLeftSWPMOV(NINODE_SWORD_LEFT_SWP_MOV),
        m_weaponAxe(NINODE_AXE),
        m_weaponAxeLeft(NINODE_AXE_LEFT),
        m_weaponAxeOnBackMOV(NINODE_AXE_BACK_MOV),
        m_weaponAxeLeftOnBackMOV(NINODE_AXE_BACK_LEFT_MOV),
        m_weaponDagger(NINODE_DAGGER),
        m_weaponDaggerLeft(NINODE_DAGGER_LEFT),
        m_weaponDaggerBackHipMOV(NINODE_DAGGER_BACK_HIP_MOV),
        m_weaponDaggerLeftBackHipMOV(NINODE_DAGGER_BACK_HIP_LEFT_MOV),
        m_weaponDaggerAnkleMOV(NINODE_DAGGER_ANKLE_MOV),
        m_weaponDaggerLeftAnkleMOV(NINODE_DAGGER_ANKLE_LEFT_MOV)
#endif
    {
    }
}