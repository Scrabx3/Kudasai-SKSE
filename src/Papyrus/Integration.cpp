#pragma once

namespace Papyrus
{
	namespace Integration
	{
		void Configuration::ReadSettings()
		{
			const auto path = fs::path{ "Data/SKSE/Plugins/Kudasai/Settings.json" };
			if (fs::exists(path) == false) {
				logger::critical("File \"settings.json\" in \"Data/SKSE/Plugins/Kudasai\" not found");
				Kudasai::WinMsgFATAL("File \"Settings.json\" in \"Data/SKSE/Plugins/Kudasai\" not found. It is essential that this file is present, Kudasai can't work without it.\n\n\rExit game now?", "Kudasai <Settings Loader>");
				return;
			}
			Json::Value file;
			std::ifstream(path.string(), std::ifstream::binary) >> file;

			settings.active = file["active"].asBool();
		}

		bool Configuration::isstripprotec(RE::TESObjectARMO* a_armor)
		{
			if (a_armor->HasKeyword(0x0A8668))	// daedric artifact
				return true;
			return excludedarmor.contains(a_armor->GetFormID());
		}

		bool Configuration::isexcludedactor(RE::Actor* subject)
		{
			return excludedactor.contains(subject->GetFormID());
		}

		bool Configuration::isactorinterested(RE::Actor* primum, RE::Actor* secundum)
		{
			return primum != nullptr && secundum != nullptr;
		}

		void Configuration::createassault(RE::Actor* primum, std::vector<RE::Actor*> secundi)
		{
			const auto skyrimVM = RE::SkyrimVM::GetSingleton();
			auto vm = skyrimVM ? skyrimVM->impl : nullptr;
			if (vm) {
				RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
				auto args = RE::MakeFunctionArguments(std::move(primum), std::move(secundi));
				vm->DispatchStaticCall("osfcore", "SetupScene", args, callback);
				delete args;
			}
		}

	}  // namespace Integration
}  // namespace Kuasai