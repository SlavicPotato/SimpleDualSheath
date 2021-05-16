#include "pch.h"

#include "Main.h"

#include "Controller.h"
#include "Config.h"
#include "EngineExtensions.h"

namespace SDS
{
    std::shared_ptr<Controller> s_controller;

    static void MessageHandler(SKSEMessagingInterface::Message* a_message)
    {
        if (a_message->type == SKSEMessagingInterface::kMessage_DataLoaded)
        {
            s_controller->InitializeData();

            auto edl = GetEventDispatcherList();

            edl->objectLoadedDispatcher.AddEventSink(s_controller.get());
            edl->initScriptDispatcher.AddEventSink(s_controller.get());
        }
    }

    bool Initialize()
    {
        Config config;
        if (!config.Load(PLUGIN_INI_FILE)) {
            gLog.Warning("Unable to load the configuration file, using defaults");
        }

        s_controller = std::make_unique<Controller>(config);

        auto& skse = ISKSE::GetSingleton();

        auto mif = skse.GetInterface<SKSEMessagingInterface>();

        auto NiNodeUpdate = static_cast<EventDispatcher<SKSENiNodeUpdateEvent>*>(mif->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_NiNodeUpdateEvent));
        if (!NiNodeUpdate) {
            gLog.Error("Could not get NiNodeUpdateEvent dispatcher");
            return false;
        }

        auto ActionEvent = static_cast<EventDispatcher<SKSEActionEvent>*>(mif->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_ActionEvent));
        if (!ActionEvent) {
            gLog.Error("Could not get ActionEvent dispatcher");
            return false;
        }

        auto CameraEvent = static_cast<EventDispatcher<SKSECameraEvent>*>(mif->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_CameraEvent));
        if (!CameraEvent) {
            gLog.Error("Could not get CameraEvent dispatcher");
            return false;
        }

        if (!mif->RegisterListener(skse.GetPluginHandle(), "SKSE", MessageHandler)) {
            gLog.Error("Failed to register SKSE message listener");
            return false;
        }

        EngineExtensions::Initialize(s_controller);

        NiNodeUpdate->AddEventSink(s_controller.get());
        ActionEvent->AddEventSink(s_controller.get());
        CameraEvent->AddEventSink(s_controller.get());

        return true;
    }
}