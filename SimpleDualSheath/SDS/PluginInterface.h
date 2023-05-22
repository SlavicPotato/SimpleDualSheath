#pragma once

#include <ext/Events.h>

namespace SDS
{
	class Controller;

	class PluginInterface :
		PluginInterfaceBase
	{
	public:

		static constexpr auto UNIQUE_ID = stl::fnv1a_64::hash_string(PLUGIN_AUTHOR "_" PLUGIN_NAME);

		PluginInterface(const stl::smart_ptr<Controller>& a_controller);

		virtual std::uint32_t GetPluginVersion() const override;
		virtual const char*   GetPluginName() const override;
		virtual std::uint32_t GetInterfaceVersion() const override;
		virtual const char*   GetInterfaceName() const override;
		virtual std::uint64_t GetUniqueID() const override;

		//

		virtual bool GetShieldOnBackEnabled(Actor* a_actor) const;
		virtual void RegisterForPlayerShieldOnBackEvent(::Events::EventSink<SDSPlayerShieldOnBackSwitchEvent>* a_sink);
		virtual bool IsWeaponNodeSharingDisabled() const;

	private:
		const stl::smart_ptr<Controller> m_controller;
	};
}
