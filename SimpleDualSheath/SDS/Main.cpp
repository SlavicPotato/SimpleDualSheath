#include "pch.h"

#include "Main.h"

#include "Controller.h"
#include "Config.h"
#include "EngineExtensions.h"

namespace SDS
{
    static std::shared_ptr<Controller> s_controller;

    static void MessageHandler(SKSEMessagingInterface::Message* a_message)
    {
        if (a_message->type == SKSEMessagingInterface::kMessage_DataLoaded)
        {
            s_controller->InitializeData();

            auto edl = GetEventDispatcherList();

            edl->objectLoadedDispatcher.AddEventSink(s_controller.get());
            edl->initScriptDispatcher.AddEventSink(s_controller.get());

            if ((s_controller->GetConfig().m_shield & Data::Flags::kEnabled) != Data::Flags::kNone)
            {
                edl->equipDispatcher.AddEventSink(s_controller.get());
            }
        }
    }

    bool Initialize(const SKSEInterface* a_skse)
    {
        Config config(PLUGIN_INI_FILE);
        if (!config.IsLoaded()) {
            gLog.Warning("Unable to load the configuration file, using defaults");
        }

        auto controller = std::make_shared<Controller>(config);

        if (!EngineExtensions::ValidateMemory(controller.get())) {
            gLog.FatalError("Memory validation failed, aborting");
            return false;
        }

        auto& skse = ISKSE::GetSingleton();

        auto mif = skse.GetInterface<SKSEMessagingInterface>();

        auto NiNodeUpdate = static_cast<EventDispatcher<SKSENiNodeUpdateEvent>*>(mif->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_NiNodeUpdateEvent));
        if (!NiNodeUpdate) {
            gLog.FatalError("Could not get NiNodeUpdateEvent dispatcher");
            return false;
        }

        auto ActionEvent = static_cast<EventDispatcher<SKSEActionEvent>*>(mif->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_ActionEvent));
        if (!ActionEvent) {
            gLog.FatalError("Could not get ActionEvent dispatcher");
            return false;
        }

        auto CameraEvent = static_cast<EventDispatcher<SKSECameraEvent>*>(mif->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_CameraEvent));
        if (!CameraEvent) {
            gLog.FatalError("Could not get CameraEvent dispatcher");
            return false;
        }

        if (!skse.CreateTrampolines(a_skse)) {
            return false;
        }

        mif->RegisterListener(skse.GetPluginHandle(), "SKSE", MessageHandler);

        EngineExtensions::Initialize(controller);

        NiNodeUpdate->AddEventSink(controller.get());
        ActionEvent->AddEventSink(controller.get());
        CameraEvent->AddEventSink(controller.get());

        s_controller = std::move(controller);

        return true;
    }
}