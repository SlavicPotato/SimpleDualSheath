#include "pch.h"

#include "Logging.h"

#include <ext/IOS.h>

namespace SDS
{
	namespace Util
	{
		namespace Logging
		{
			void AbortPopupWrite(const char* a_message)
			{
				gLog.FatalError("%s", a_message);
				WinApi::MessageBoxError(PLUGIN_NAME, a_message);
			}

			void AbortPopup(const char* a_message)
			{
				WinApi::MessageBoxError(PLUGIN_NAME, a_message);
			}
		}
	}
}