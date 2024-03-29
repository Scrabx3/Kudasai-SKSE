#include "Kudasai/Combat/Hooks.h"
#include "Kudasai/Combat/Resolution.h"
#include "Kudasai/EventSink.h"
#include "Kudasai/Interface/QTE.h"
#include "Papyrus/Events.h"
#include "Papyrus/Functions.h"
#include "Papyrus/Settings.h"

bool InitLogger()
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path)
		return false;

	*path /= fmt::format(FMT_STRING("{}.log"), Plugin::NAME);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
	log->flush_on(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%s(%#): [%^%l%$] %v"s);

	logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());

	return true;
}

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message)
{
	switch (message->type) {
	case SKSE::MessagingInterface::kSaveGame:
		Kudasai::Resolution::GetSingleton()->UpdateWeights();
		break;
	case SKSE::MessagingInterface::kDataLoaded:
		Papyrus::Configuration::Data::GetSingleton()->LoadData();
		Kudasai::Resolution::GetSingleton()->Register();
		Serialization::FormDeletionHandler::Register();
		break;
	case SKSE::MessagingInterface::kNewGame:
	case SKSE::MessagingInterface::kPostLoadGame:
		Kudasai::Resolution::GetSingleton()->UpdateProperties();
		Papyrus::Settings::GetSingleton()->UpdateSettings();
		break;
	}
}

#ifdef SKYRIM_SUPPORT_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Plugin::VERSION);
	v.PluginName(Plugin::NAME);
	v.AuthorName("Scrab Joséline"sv);
	v.UsesAddressLibrary();
	v.UsesUpdatedStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });
	return v;
}();
#else
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	if (!InitLogger())
		return false;

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION.pack();

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}
	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39 || ver > SKSE::RUNTIME_LATEST) {
		logger::critical("Unsupported runtime version {}"sv, ver.string());
		return false;
	}
	return true;
}
#endif

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
#ifdef SKYRIM_SUPPORT_AE
	if (!InitLogger())
		return false;
#endif

	SKSE::Init(a_skse);

	logger::info("{} loaded"sv, Plugin::NAME);

	const auto papyrus = SKSE::GetPapyrusInterface();
	papyrus->Register(Papyrus::RegisterFuncs);
	papyrus->Register(Papyrus::RegisterEvents);

	const auto msging = SKSE::GetMessagingInterface();
	if (!msging->RegisterListener("SKSE", SKSEMessageHandler)) {
		logger::critical("Failed to register Listener");
		return false;
	}

	Kudasai::Interface::QTE::Register();
	Kudasai::EventHandler::Register();
	Kudasai::Hooks::InstallHook();

	const auto serialization = SKSE::GetSerializationInterface();
	serialization->SetUniqueID('YKud');
	serialization->SetSaveCallback(Serialization::Serialize::SaveCallback);
	serialization->SetLoadCallback(Serialization::Serialize::LoadCallback);
	serialization->SetRevertCallback(Serialization::Serialize::RevertCallback);
	serialization->SetFormDeleteCallback(Serialization::Serialize::FormDeleteCallback);


	logger::info("Initialization complete");

	return true;
}
