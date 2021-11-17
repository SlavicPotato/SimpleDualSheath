#include "pch.h"

#include "Main.h"

#include "Config.h"
#include "Controller.h"
#include "EngineExtensions.h"
#include <ext/SKSEMessaging.h>

namespace SDS
{
	static std::shared_ptr<Controller> s_controller;

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
					auto evd = InputEventDispatcher::GetSingleton();
					if (evd)
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

				auto edl = GetEventDispatcherList();
				if (edl)
				{
					edl->objectLoadedDispatcher.AddEventSink(s_controller.get());
					edl->initScriptDispatcher.AddEventSink(s_controller.get());

					auto& config = s_controller->GetConfig();

					if (config.m_npcEquipLeft)
					{
						edl->equipDispatcher.AddEventSink(s_controller.get());
						edl->containerChangedDispatcher.AddEventSink(s_controller.get());
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
		case SKSEMessagingInterface::kMessage_NewGame:
			s_loaded = true;
			break;
		case SKSEMessagingInterface::kMessage_PostPostLoad:
			{
				auto& skse = ISKSE::GetSingleton();
				auto mi = skse.GetInterface<SKSEMessagingInterface>();

				using namespace SKSEMessaging;

				NodeListMessage data;

				std::vector<NodeListMessage::Entry> tmp;

				auto& config = s_controller->GetConfig();

				if (config.m_sword.IsEnabled() && !config.m_sword.m_sheathNode.empty())
				{
					tmp.emplace_back(config.m_sword.m_sheathNode.c_str());
				}

				if (config.m_axe.IsEnabled() && !config.m_axe.m_sheathNode.empty())
				{
					tmp.emplace_back(config.m_axe.m_sheathNode.c_str());
				}

				if (config.m_mace.IsEnabled() && !config.m_mace.m_sheathNode.empty())
				{
					tmp.emplace_back(config.m_mace.m_sheathNode.c_str());
				}

				if (config.m_dagger.IsEnabled() && !config.m_dagger.m_sheathNode.empty())
				{
					tmp.emplace_back(config.m_dagger.m_sheathNode.c_str());
				}

				if (config.m_staff.IsEnabled() && !config.m_staff.m_sheathNode.empty())
				{
					tmp.emplace_back(config.m_staff.m_sheathNode.c_str());
					tmp.emplace_back(StringHolder::NINODE_STAFF);
				}

				if (config.m_2hSword.IsEnabled() && !config.m_2hSword.m_sheathNode.empty())
				{
					tmp.emplace_back(config.m_2hSword.m_sheathNode.c_str());
				}

				if (config.m_2hAxe.IsEnabled() && !config.m_2hAxe.m_sheathNode.empty())
				{
					tmp.emplace_back(config.m_2hAxe.m_sheathNode.c_str());
				}

				/*if (config.m_shield.IsEnabled() && !config.m_shield.m_sheathNode.empty())
                {
                    tmp.emplace_back(config.m_shield.m_sheathNode.c_str());
                }*/

				data.list = tmp.data();
				data.size = static_cast<std::uint32_t>(tmp.size());

				mi->Dispatch(
					skse.GetPluginHandle(),
					SKSEMessaging::NodeListMessage::kMessage_NodeList,
					static_cast<void*>(std::addressof(data)),
					sizeof(data),
					nullptr);
			}
			break;
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
		EngineExtensions::MemoryValidationFlags a_flags)
	{
		using flag_t = EngineExtensions::MemoryValidationFlags;

		std::string result;

		if ((a_flags & flag_t::kWeaponLeftAttach) == flag_t::kWeaponLeftAttach)
		{
			result += "WeaponLeftAttach, ";
		}

		if ((a_flags & flag_t::kStaffAttach) == flag_t::kStaffAttach)
		{
			result += "StaffAttach, ";
		}

		if ((a_flags & flag_t::kShieldAttach) == flag_t::kShieldAttach)
		{
			result += "ShieldAttach, ";
		}

		if ((a_flags & flag_t::kDisableShieldHideOnSit) == flag_t::kDisableShieldHideOnSit)
		{
			result += "DisableShieldHideOnSit, ";
		}

		if ((a_flags & flag_t::kScabbardAttach) == flag_t::kScabbardAttach)
		{
			result += "ScabbardAttach, ";
		}

		if ((a_flags & flag_t::kScabbardDetach) == flag_t::kScabbardDetach)
		{
			result += "ScabbardDetach";
		}

		if ((a_flags & flag_t::kScabbardGet) == flag_t::kScabbardGet)
		{
			result += "ScabbardGet";
		}

		StrHelpers::rtrim(result, ", ");

		return result;
	}

	bool Initialize(const SKSEInterface* a_skse)
	{
		Config config(PLUGIN_INI_FILE);
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

#ifdef _SDS_UNUSED
		auto NiNodeUpdate = static_cast<EventDispatcher<SKSENiNodeUpdateEvent>*>(mif->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_NiNodeUpdateEvent));
		if (!NiNodeUpdate)
		{
			gLog.FatalError("Could not get NiNodeUpdateEvent dispatcher");
			return false;
		}
#endif

		auto ActionEvent = static_cast<EventDispatcher<SKSEActionEvent>*>(mif->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_ActionEvent));
		if (!ActionEvent)
		{
			gLog.FatalError("Could not get ActionEvent dispatcher");
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

#ifdef _SDS_UNUSED
		NiNodeUpdate->AddEventSink(controller.get());
#endif
		ActionEvent->AddEventSink(controller.get());

		s_controller = std::move(controller);

		return true;
	}
}