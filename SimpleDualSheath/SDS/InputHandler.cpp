#include "pch.h"

#include "InputHandler.h"

namespace SDS
{
	auto InputHandler::ReceiveEvent(
		InputEvent* const* a_evns,
		BSTEventSource<InputEvent*>*)
		-> EventResult
	{
		if (a_evns)
		{
			for (auto inputEvent = *a_evns; inputEvent; inputEvent = inputEvent->next)
			{
				auto buttonEvent = inputEvent->AsButtonEvent();
				if (!buttonEvent)
				{
					continue;
				}

				if (buttonEvent->device != INPUT_DEVICE::kKeyboard)
				{
					continue;
				}

				std::uint32_t keyCode = buttonEvent->GetIDCode();

				if (!keyCode || keyCode >= InputMap::kMaxMacros)
				{
					continue;
				}

				if (buttonEvent->IsDown())
				{
					OnKeyDown(keyCode);
				}
				else if (buttonEvent->IsUpLF())
				{
					OnKeyUp(keyCode);
				}
			}
		}

		return EventResult::kContinue;
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
		SetComboKey(a_comboKey);
		SetKey(a_key);
	}

	void ComboKeyPressHandler::OnKeyDown(std::uint32_t a_keyCode)
	{
		if (m_comboKey && a_keyCode == m_comboKey)
		{
			m_comboKeyDown = true;
		}

		if (m_key && a_keyCode == m_key && (!m_comboKey || m_comboKeyDown))
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