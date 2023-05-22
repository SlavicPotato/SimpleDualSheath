#include "pch.h"

#include "Controller.h"
#include "PluginInterface.h"

namespace SDS
{

	PluginInterface::PluginInterface(
		const stl::smart_ptr<Controller>& a_controller) :
		m_controller(a_controller)
	{
	}

	std::uint32_t PluginInterface::GetPluginVersion() const
	{
		return MAKE_PLUGIN_VERSION(
			PLUGIN_VERSION_MAJOR,
			PLUGIN_VERSION_MINOR,
			PLUGIN_VERSION_REVISION);
	}

	std::uint32_t PluginInterface::GetInterfaceVersion() const
	{
		return 2;
	}

	const char* PluginInterface::GetPluginName() const
	{
		return PLUGIN_NAME;
	}

	const char* PluginInterface::GetInterfaceName() const
	{
		return "Main";
	}

	std::uint64_t PluginInterface::GetUniqueID() const
	{
		return UNIQUE_ID;
	}

	bool PluginInterface::GetShieldOnBackEnabled(Actor* a_actor) const
	{
		if (!m_controller->IsShieldEnabled(a_actor))
		{
			return false;
		}

		return m_controller->GetShieldOnBackSwitch(a_actor);
	}

	void PluginInterface::RegisterForPlayerShieldOnBackEvent(
		::Events::EventSink<SDSPlayerShieldOnBackSwitchEvent>* a_sink)
	{
		if (a_sink)
		{
			m_controller->AddSink(a_sink);
		}
	}

	bool PluginInterface::IsWeaponNodeSharingDisabled() const
	{
		return m_controller->GetConfig().m_disableWeapNodeSharing;
	}
}