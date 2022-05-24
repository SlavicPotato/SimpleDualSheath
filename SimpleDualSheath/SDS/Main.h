#pragma once

namespace SDS
{
	class PluginInterface;

	extern bool Initialize(const SKSEInterface* a_skse);
	extern PluginInterface* GetPluginInterface();
}