#pragma once

#pragma warning(push)
#pragma warning(disable : 4200)
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#pragma warning(pop)

#include <unordered_map>
#include <atomic>
#include <yaml-cpp/yaml.h>

#pragma warning(push)
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#pragma warning(pop)

namespace logger = SKSE::log;
namespace fs = std::filesystem;
using namespace std::literals;

#include "Random.h"
#include "Singleton.h"
#include "Kudasai/Misc.h"
#include "Serialization/Serialize.h"

#define ESPNAME "YameteKudasai.esp"
static constexpr auto CONFIGPATH = [](std::string file) -> std::string { return "Data\\SKSE\\Kudasai\\"s + file; };

#ifdef SKYRIM_SUPPORT_AE
#define RELID(SE, AE) REL::ID(AE)
#define OFFSET(SE, AE) AE
#else
#define RELID(SE, AE) REL::ID(SE)
#define OFFSET(SE, AE) SE
#endif

using Serialize = Serialization::Serialize;
namespace stl
{
	using namespace SKSE::stl;
}

#define DLLEXPORT __declspec(dllexport)
#include "Plugin.h"
