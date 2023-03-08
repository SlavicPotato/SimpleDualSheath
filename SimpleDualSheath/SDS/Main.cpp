#include "pch.h"

#include "Main.h"

#include "Config.h"
#include "Controller.h"
#include "EngineExtensions.h"
#include "PluginInterface.h"

#include <ext/SKSEMessaging.h>

namespace SDS
{
	static std::shared_ptr<Controller>      s_controller;
	static std::unique_ptr<PluginInterface> s_pluginInterface;

	static bool s_loaded = false;

	static void MessageHandler(SKSEMessagingInterface::Message* a_message)
	{
		switch (a_message->type)
		{
		case SKSEMessagingInterface::kMessage_InputLoaded:
			{
				auto& config = s_controller->GetConfig();

				if (config.m_shieldToggleKeys.Has() &&
				    config.m_shield.IsPlayerEnabled())
				{
					if (auto evd = InputEventDispatcher::GetSingleton())
					{
						s_controller->SetKeys(
							config.m_shieldToggleKeys.GetComboKey(),
							config.m_shieldToggleKeys.GetKey());

						evd->AddEventSink(s_controller.get());
					}
					else
					{
						gLog.Error("Couldn't get input event dispatcher");
					}
				}
			}
			break;
		case SKSEMessagingInterface::kMessage_DataLoaded:
			{
				s_controller->InitializeData();

				if (auto edl = ScriptEventSourceHolder::GetSingleton())
				{
					edl->AddEventSink<TESObjectLoadedEvent>(s_controller.get());
					edl->AddEventSink<TESInitScriptEvent>(s_controller.get());
					//edl->AddEventSink<TESSwitchRaceCompleteEvent>(s_controller.get());

					auto& config = s_controller->GetConfig();

					if (config.m_npcEquipLeft)
					{
						edl->AddEventSink<TESEquipEvent>(s_controller.get());
						edl->AddEventSink<TESContainerChangedEvent>(s_controller.get());
					}
				}
				else
				{
					gLog.Error("Couldn't get event dispatcher list");
				}
			}
			break;
		case SKSEMessagingInterface::kMessage_PostLoadGame:
			// skip first, evaluate on subsequent loads
			if (s_loaded)
			{
				s_controller->EvaluateDrawnStateOnNearbyActors();
			}
			[[fallthrough]];
		case SKSEMessagingInterface::kMessage_NewGame:
			s_loaded = true;
			break;
		case SKSEMessagingInterface::kMessage_PostLoad:
			if (s_controller)
			{
				s_pluginInterface = std::make_unique<PluginInterface>(s_controller);
			}
		}
	}

	static void SaveGameHandler(SKSESerializationInterface* a_intfc)
	{
		s_controller->SaveGameHandler(a_intfc);
	}

	static void LoadGameHandler(SKSESerializationInterface* a_intfc)
	{
		s_controller->LoadGameHandler(a_intfc);
	}

	static std::string MVResultToString(
		stl::flag<EngineExtensions::MemoryValidationFlags> a_flags)
	{
		using flag_t = EngineExtensions::MemoryValidationFlags;

		std::string result;

		if (a_flags.test(flag_t::kWeaponLeftAttach))
		{
			result += "WeaponLeftAttach, ";
		}

		if (a_flags.test(flag_t::kStaffAttach))
		{
			result += "StaffAttach, ";
		}

		if (a_flags.test(flag_t::kShieldAttach))
		{
			result += "ShieldAttach, ";
		}

		if (a_flags.test(flag_t::kDisableShieldHideOnSit))
		{
			result += "DisableShieldHideOnSit, ";
		}

		if (a_flags.test(flag_t::kScabbardAttach))
		{
			result += "ScabbardAttach, ";
		}

		if (a_flags.test(flag_t::kScabbardDetach))
		{
			result += "ScabbardDetach, ";
		}

		if (a_flags.test(flag_t::kScabbardGet))
		{
			result += "ScabbardGet";
		}

		stl::rtrim(result, ", ");

		return result;
	}

	bool Initialize(const SKSEInterface* a_skse)
	{
		Config config(PLUGIN_INI_FILE_NOEXT);
		if (!config.IsLoaded())
		{
			gLog.Warning("Unable to load the configuration file, using defaults");
		}

		if (!ITaskPool::ValidateMemory())
		{
			gLog.FatalError("ITaskPool: memory validation failed");
			return false;
		}

		auto mvResult = EngineExtensions::ValidateMemory(config);

		if (mvResult != EngineExtensions::MemoryValidationFlags::kNone)
		{
			auto desc = MVResultToString(mvResult);
			gLog.FatalError("Memory validation failed (%s), aborting", desc.c_str());
			return false;
		}

		auto controller = std::make_shared<Controller>(config);

		auto& skse = ISKSE::GetSingleton();

		auto mif = skse.GetInterface<SKSEMessagingInterface>();

		/*auto nnupd_evd = mif->GetEventDispatcher<SKSENiNodeUpdateEvent>();
		if (!nnupd_evd)
		{
			gLog.Warning("Could not get NiNodeUpdateEvent dispatcher");
		}*/

		auto aed = mif->GetEventDispatcher<SKSEActionEvent>();
		if (!aed)
		{
			gLog.FatalError("Could not get SKSEActionEvent dispatcher");
			return false;
		}

		if (!skse.CreateTrampolines(a_skse))
		{
			return false;
		}

		ITaskPool::Install(
			ISKSE::GetBranchTrampoline(),
			ISKSE::GetLocalTrampoline());

		mif->RegisterListener(skse.GetPluginHandle(), "SKSE", MessageHandler);

		auto si = skse.GetInterface<SKSESerializationInterface>();

		si->SetUniqueID(skse.GetPluginHandle(), 'ASDS');
		si->SetSaveCallback(skse.GetPluginHandle(), SaveGameHandler);
		si->SetLoadCallback(skse.GetPluginHandle(), LoadGameHandler);

		EngineExtensions::Initialize(controller);

		//nnupd_evd->AddEventSink(controller.get());
		aed->AddEventSink(controller.get());

		s_controller = controller;

		return true;
	}

	PluginInterface* GetPluginInterface()
	{
		return s_pluginInterface.get();
	}
}