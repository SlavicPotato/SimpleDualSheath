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

            auto& config = s_controller->GetConfig();

            if (config.m_npcEquipLeft)
            {
                edl->equipDispatcher.AddEventSink(s_controller.get());
                edl->containerChangedDispatcher.AddEventSink(s_controller.get());
            }
        }
    }

    static std::string MVResultToString(
        EngineExtensions::MemoryValidationFlags a_flags)
    {
        using flag_t = EngineExtensions::MemoryValidationFlags;
        using flag_size_t = typename std::underlying_type<flag_t>::type;

        std::string result;

        if ((a_flags & flag_t::kCreateArmorNode) == flag_t::kCreateArmorNode) {
            result += "CreateArmorNode, ";
        }
        
        if ((a_flags & flag_t::kCreateWeaponNodes) == flag_t::kCreateWeaponNodes) {
            result += "CreateWeaponNodes, ";
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

        /*auto CameraEvent = static_cast<EventDispatcher<SKSECameraEvent>*>(mif->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_CameraEvent));
        if (!CameraEvent) {
            gLog.FatalError("Could not get CameraEvent dispatcher");
            return false;
        }*/

        if (!skse.CreateTrampolines(a_skse)) {
            return false;
        }

        mif->RegisterListener(skse.GetPluginHandle(), "SKSE", MessageHandler);

        EngineExtensions::Initialize(controller);

#ifdef _SDS_UNUSED
        NiNodeUpdate->AddEventSink(controller.get());
#endif
        ActionEvent->AddEventSink(controller.get());
        //CameraEvent->AddEventSink(controller.get());

        s_controller = std::move(controller);

        return true;
    }
}