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
		m_scbLeft(NINODE_SCB_LEFT),
		//m_weaponBack(NINODE_WEAPON_BACK),
		m_weaponBackAxeMace(NINODE_WEAPON_BACK_AXE_MACE),
		//m_weaponBow(NINODE_BOW),
		m_weaponCrossbow(NINODE_CROSSBOW)
	{
	}
}