#include "pch.h"

#include "StringHolder.h"

namespace SDS
{
    StringHolder::StringHolder() :
        m_shield(NINODE_SHIELD),
        m_weapon(NINODE_WEAPON),
        m_npcroot(NINODE_NPCROOT),
        m_shieldSheathNode(NINODE_SHIELD_BACK),
        m_iLeftHandType(iLeftHandType),
        m_iLeftHandEquipped(iLeftHandEquipped),
        m_scbLeft(NINODE_SCB_LEFT)
#ifdef _SDS_UNUSED
        ,
        m_weaponSword(NINODE_SWORD),
        m_weaponSwordLeft(NINODE_SWORD_LEFT),
        m_weaponSwordOnBackMOV(NINODE_SWORD_BACK_MOV),
        m_weaponSwordLeftOnBackMOV(NINODE_SWORD_LEFT_BACK_MOV),
        m_weaponSwordSWPMOV(NINODE_SWORD_SWP_MOV),
        m_weaponSwordLeftSWPMOV(NINODE_SWORD_LEFT_SWP_MOV),
        m_weaponSwordDefaultMOV(NINODE_SWORD_DEFAULT_MOV),
        m_weaponSwordLeftDefaultMOV(NINODE_SWORD_LEFT_DEFAULT_MOV),
        m_weaponAxe(NINODE_AXE),
        m_weaponAxeLeft(NINODE_AXE_LEFT),
        m_weaponAxeOnBackMOV(NINODE_AXE_BACK_MOV),
        m_weaponAxeLeftOnBackMOV(NINODE_AXE_BACK_LEFT_MOV),
        m_weaponAxeDefaultMOV(NINODE_AXE_DEFAULT_MOV),
        m_weaponAxeLeftDefaultMOV(NINODE_AXE_LEFT_DEFAULT_MOV),
        m_weaponDagger(NINODE_DAGGER),
        m_weaponDaggerLeft(NINODE_DAGGER_LEFT),
        m_weaponDaggerBackHipMOV(NINODE_DAGGER_BACK_HIP_MOV),
        m_weaponDaggerLeftBackHipMOV(NINODE_DAGGER_BACK_HIP_LEFT_MOV),
        m_weaponDaggerAnkleMOV(NINODE_DAGGER_ANKLE_MOV),
        m_weaponDaggerLeftAnkleMOV(NINODE_DAGGER_ANKLE_LEFT_MOV),
        m_weaponDaggerDefaultMOV(NINODE_DAGGER_DEFAULT_MOV),
        m_weaponDaggerLeftDefaultMOV(NINODE_DAGGER_LEFT_DEFAULT_MOV)
#endif
    {
    }
}