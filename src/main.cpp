#include "Kudasai/Combat/Hooks.h"
#include "Papyrus/Functions.h"
#include "Serialization/Storage.h"
#include "Kudasai/Interface/QTE.h"

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= "YameteKudasai.log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::trace);
	log->flush_on(spdlog::level::trace);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%l] %v"s);

	logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = stl::version_pack(Plugin::VERSION);

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39) {
		logger::critical("Unsupported runtime version {}"sv, ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);

	logger::info("{} loaded"sv, Plugin::NAME);

	auto papyrus = SKSE::GetPapyrusInterface();
	if (!papyrus->Register(Papyrus::RegisterFuncs)) {
		logger::critical("Failed to register Papyrus Scripts");
		return false;
	}
	Kudasai::Interface::QTE::Register();
	Kudasai::Hooks::InstallHook();

	auto serialization = SKSE::GetSerializationInterface();
	serialization->SetUniqueID('YKud');
	serialization->SetSaveCallback(Serialize::SaveCallback);
	serialization->SetLoadCallback(Serialize::LoadCallback);
	serialization->SetRevertCallback(Serialize::RevertCallback);

	logger::info("Initialization complete");

	return true;
}
