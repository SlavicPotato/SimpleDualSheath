#include "pch.h"

#include "SDS/Main.h"
#include "SDS/Util/Logging.h"

static bool Initialize(const SKSEInterface* a_skse)
{
	if (!IAL::IsAE())
	{
		if (!IAL::IsLoaded())
		{
			gLog.FatalError("Could not load the address library");
			return false;
		}

		if (IAL::HasBadQuery())
		{
			gLog.FatalError("One or more addresses could not be retrieved from the database");
			return false;
		}
	}

	auto& skse = ISKSE::GetSingleton();

	if (!skse.QueryInterfaces(a_skse))
	{
		return false;
	}

	auto ret = SDS::Initialize(a_skse);

	if (ret)
	{
		auto usageBranch = skse.GetTrampolineUsage(TrampolineID::kBranch);
		auto usageLocal = skse.GetTrampolineUsage(TrampolineID::kLocal);

		gLog.Message("Loaded, trampolines: branch:[%zu/%zu] codegen:[%zu/%zu]", usageBranch.used, usageBranch.total, usageLocal.used, usageLocal.total);
	}

	return ret;
}

extern "C" {
	bool SKSEPlugin_Query(const SKSEInterface* a_skse, PluginInfo* a_info)
	{
		return ISKSE::GetSingleton().Query(a_skse, a_info);
	}

	bool SKSEPlugin_Load(const SKSEInterface* a_skse)
	{
		using namespace SDS::Util::Logging;

		if (IAL::IsAE())
		{
			auto& iskse = ISKSE::GetSingleton();

			iskse.SetPluginHandle(a_skse->GetPluginHandle());
			iskse.OpenLog();
		}

		gLog.Message("Initializing %s version %s (runtime %u.%u.%u.%u)", PLUGIN_NAME, PLUGIN_VERSION_VERSTRING, GET_EXE_VERSION_MAJOR(a_skse->runtimeVersion), GET_EXE_VERSION_MINOR(a_skse->runtimeVersion), GET_EXE_VERSION_BUILD(a_skse->runtimeVersion), GET_EXE_VERSION_SUB(a_skse->runtimeVersion));

		bool ret = Initialize(a_skse);

		if (!ret)
		{
			AbortPopup(
				"Plugin initialization failed\n\n"
				"See log for more information");
		}

		IAL::Release();
		gLog.Close();

		return ret;
	}

	__declspec(dllexport) SKSEPluginVersionData SKSEPlugin_Version = {
		SKSEPluginVersionData::kVersion,

		MAKE_PLUGIN_VERSION(
			PLUGIN_VERSION_MAJOR,
			PLUGIN_VERSION_MINOR,
			PLUGIN_VERSION_REVISION),
		PLUGIN_NAME,
		PLUGIN_AUTHOR,
		"n/a",
		0,
		{ RUNTIME_VERSION_1_6_318, 0 },
		0,
	};
};