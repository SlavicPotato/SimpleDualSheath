#include "pch.h"

#include "SDS/Main.h"

static bool Initialize(const SKSEInterface* a_skse)
{
    if (!IAL::IsLoaded()) {
        gLog.FatalError("Could not load the address library");
        return false;
    }

    if (IAL::HasBadQuery()) {
        gLog.FatalError("One or more addresses could not be retrieved from the database");
        return false;
    }

    auto& skse = ISKSE::GetSingleton();

    if (!skse.Initialize(a_skse)) {
        return false;
    }

    auto ret = SDS::Initialize();

    if (ret) 
    {
        auto usageBranch = skse.GetTrampolineUsage(TrampolineID::kBranch);
        auto usageLocal = skse.GetTrampolineUsage(TrampolineID::kLocal);

        gLog.Message("Loaded OK, trampolines: branch:[%zu/%zu] codegen:[%zu/%zu]", 
            usageBranch.used, usageBranch.total, usageLocal.used, usageLocal.total);
    }
    else {
        gLog.FatalError("Initialization failed!");
    }

    return ret;
}

extern "C"
{
    bool SKSEPlugin_Query(const SKSEInterface* a_skse, PluginInfo* a_info)
    {
        return ISKSE::GetSingleton().Query(a_skse, a_info);
    }

    bool SKSEPlugin_Load(const SKSEInterface* a_skse)
    {
        gLog.Message("Initializing %s version %s (runtime %u.%u.%u.%u)",
            PLUGIN_NAME, PLUGIN_VERSION_VERSTRING,
            GET_EXE_VERSION_MAJOR(a_skse->runtimeVersion),
            GET_EXE_VERSION_MINOR(a_skse->runtimeVersion),
            GET_EXE_VERSION_BUILD(a_skse->runtimeVersion),
            GET_EXE_VERSION_SUB(a_skse->runtimeVersion));

        bool ret = Initialize(a_skse);

        IAL::Release();
        gLog.Close();

        return ret;
    }
};