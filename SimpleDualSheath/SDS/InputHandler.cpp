#include "pch.h"

#include "InputHandler.h"

namespace SDS
{
	auto InputHandler::ReceiveEvent(InputEvent** evns, InputEventDispatcher* dispatcher)
		-> EventResult
	{
		if (!*evns)
		{
			return kEvent_Continue;
		}

		for (auto inputEvent = *evns; inputEvent; inputEvent = inputEvent->next)
		{
			if (inputEvent->eventType != InputEvent::kEventType_Button)
				continue;

			auto buttonEvent = RTTI<ButtonEvent>()(inputEvent);
			if (!buttonEvent)
				continue;

			if (buttonEvent->deviceType != kDeviceType_Keyboard)
			{
				continue;
			}

			std::uint32_t keyCode = buttonEvent->keyMask;

			if (keyCode >= InputMap::kMaxMacros)
				continue;

			if (buttonEvent->flags != 0)
			{
				if (buttonEvent->timer == 0.0f)
				{
					OnKeyDown(keyCode);
				}
			}
			else
			{
				OnKeyUp(keyCode);
			}
		}

		return kEvent_Continue;
	}

	void ComboKeyPressHandler::SetComboKey(std::uint32_t a_key)
	{
		m_comboKey = a_key;
		m_comboKeyDown = false;
	}

	void ComboKeyPressHandler::SetKey(std::uint32_t a_key)
	{
		m_key = a_key;
	}

	void ComboKeyPressHandler::SetKeys(std::uint32_t a_comboKey, std::uint32_t a_key)
	{
		m_comboKey = a_comboKey;
		m_key = a_key;
		m_comboKeyDown = false;
	}

	void ComboKeyPressHandler::OnKeyDown(std::uint32_t a_keyCode)
	{
		if (m_comboKey && a_keyCode == m_comboKey)
		{
			m_comboKeyDown = true;
		}

		if (a_keyCode == m_key && (!m_comboKey || m_comboKeyDown))
		{
			OnKeyPressed();
		}
	}

	void ComboKeyPressHandler::OnKeyUp(std::uint32_t a_keyCode)
	{
		if (m_comboKey && a_keyCode == m_comboKey)
		{
			m_comboKeyDown = false;
		}
	}

}