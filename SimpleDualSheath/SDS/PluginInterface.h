#pragma once

#include <ext/Events.h>

namespace SDS
{
	class Controller;

	class PluginInterface :
		PluginInterfaceBase
	{
	public:

		PluginInterface(const std::shared_ptr<Controller>& a_controller);

		virtual std::uint32_t GetPluginVersion() const override;
		virtual const char*   GetPluginName() const override;
		virtual std::uint32_t GetInterfaceVersion() const override;
		virtual const char*   GetInterfaceName() const override;
		virtual std::uint64_t GetUniqueID() const override;

		//

		virtual bool GetShieldOnBackEnabled(Actor* a_actor) const;
		virtual void RegisterForPlayerShieldOnBackEvent(::Events::EventSink<SDSPlayerShieldOnBackSwitchEvent>* a_sink);

	private:
		const std::shared_ptr<Controller> m_controller;
	};
}
