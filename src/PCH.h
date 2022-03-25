#pragma once

#pragma warning(push)
#pragma warning(disable : 4200)
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#pragma warning(pop)

#include <unordered_map>
#include <atomic>
// #include <json/json.h>
#include <yaml-cpp/yaml.h>

#pragma warning(push)
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#pragma warning(pop)

namespace logger = SKSE::log;
namespace fs = std::filesystem;
using namespace std::literals;

#include "Kudasai/Misc.h"
#include "Serialization/Storage.h"

#define ESPNAME "YKudasai.esp"
static constexpr auto CONFIGPATH = [](std::string file) -> std::string { return "Data\\SKSE\\Plugins\\Kudasai\\"s + file; };

using Srl = Serialize::Storage;
namespace stl
{
	using namespace SKSE::stl;

#ifdef IS_PRE_AE
	constexpr std::uint32_t version_pack(REL::Version a_version) noexcept
	{
		return static_cast<std::uint32_t>(
			(a_version[0] & 0x0FF) << 24u |
			(a_version[1] & 0x0FF) << 16u |
			(a_version[2] & 0xFFF) << 4u |
			(a_version[3] & 0x00F) << 0u);
	}
#endif
}

#define DLLEXPORT __declspec(dllexport)
#include "Plugin.h"
