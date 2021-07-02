#include "pch.h"

#include "Main.h"

#include "Controller.h"
#include "Config.h"
#include "EngineExtensions.h"

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
                else {
                    gLog.Error("Couldn't get input event dispatcher");
                }
            }
        }
        break;
        case SKSEMessagingInterface::kMessage_DataLoaded:
        {
            s_controller->InitializeData();

            auto edl = GetEventDispatcherList();

            edl->objectLoadedDispatcher.AddEventSink(s_controller.get());
            edl->initScriptDispatcher.AddEventSink(s_controller.get());

            auto& config = s_controller->GetConfig();

            if (config.m_npcEquipLeft)
            {
                edl->equipDispatcher.AddEventSink(s_controller.get());
                edl->containerChangedDispatcher.AddEventSink(s_controller.get());
            }
        }
        break;
        case SKSEMessagingInterface::kMessage_PostLoadGame:
            // skip first, evaluate on subsequent loads
            if (s_loaded) {
                s_controller->EvaluateDrawnStateOnNearbyActors();
            }
        case SKSEMessagingInterface::kMessage_NewGame:
            s_loaded = true;
            break;
        }
    }

    static std::string MVResultToString(
        EngineExtensions::MemoryValidationFlags a_flags)
    {
        using flag_t = EngineExtensions::MemoryValidationFlags;

        std::string result;

        if ((a_flags & flag_t::kWeaponLeftAttach) == flag_t::kWeaponLeftAttach) {
            result += "WeaponLeftAttach, ";
        }

        if ((a_flags & flag_t::kStaffAttach) == flag_t::kStaffAttach) {
            result += "StaffAttach, ";
        }

        if ((a_flags & flag_t::kShieldAttach) == flag_t::kShieldAttach) {
            result += "ShieldAttach, ";
        }

        if ((a_flags & flag_t::kDisableShieldHideOnSit) == flag_t::kDisableShieldHideOnSit) {
            result += "DisableShieldHideOnSit, ";
        }

        if ((a_flags & flag_t::kScabbardAttach) == flag_t::kScabbardAttach) {
            result += "ScabbardAttach, ";
        }

        if ((a_flags & flag_t::kScabbardDetach) == flag_t::kScabbardDetach) {
            result += "ScabbardDetach";
        }

        if ((a_flags & flag_t::kScabbardGet) == flag_t::kScabbardGet) {
            result += "ScabbardGet";
        }

        StrHelpers::rtrim(result, ", ");

        return result;
    }

    bool Initialize(const SKSEInterface* a_skse)
    {
        Config config(PLUGIN_INI_FILE);
        if (!config.IsLoaded()) {
            gLog.Warning("Unable to load the configuration file, using defaults");
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
        if (!NiNodeUpdate) {
            gLog.FatalError("Could not get NiNodeUpdateEvent dispatcher");
            return false;
        }
#endif

        auto ActionEvent = static_cast<EventDispatcher<SKSEActionEvent>*>(mif->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_ActionEvent));
        if (!ActionEvent) {
            gLog.FatalError("Could not get ActionEvent dispatcher");
            return false;
        }

        if (!skse.CreateTrampolines(a_skse)) {
            return false;
        }

        if (!ITaskPool::Install(ISKSE::GetBranchTrampoline())) {
            gLog.FatalError("Failed to install task hook");
            return false;
        }

        mif->RegisterListener(skse.GetPluginHandle(), "SKSE", MessageHandler);

        EngineExtensions::Initialize(controller);

#ifdef _SDS_UNUSED
        NiNodeUpdate->AddEventSink(controller.get());
#endif
        ActionEvent->AddEventSink(controller.get());

        s_controller = std::move(controller);

        return true;
    }
}