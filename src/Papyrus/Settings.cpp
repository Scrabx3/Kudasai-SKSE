#include "Settings.h"

namespace Papyrus
{
	const bool Configuration::isstripprotec(RE::TESObjectARMO* a_armor)
	{
		const auto DaedricArtifact = RE::TESForm::LookupByID<RE::BGSKeyword>(0xA8668);
		if (a_armor->HasKeyword(DaedricArtifact))
			return true;
		auto key = createkeypair(a_armor->GetFormID());
		return excludedarmor.contains(key);
	}

	const bool Configuration::isexcludedactor(RE::Actor* subject)
	{
		logger::info("Checking exclusion on subject = {}", subject->GetFormID());
		if (subject->IsPlayerRef())
			return false;
		const auto race = subject->GetRace();
		if (!race) {
			logger::warn("Subject has no race?");
			return true;
		}
		const auto elderrace = RE::TESForm::LookupByID<RE::TESRace>(0x67CD8);
		const auto eldervamprace = RE::TESForm::LookupByID<RE::TESRace>(0xA82BA);
		const auto dlc1keeperrace = RE::TESForm::LookupByID<RE::TESRace>(0x2007AF3);
		if (race == elderrace || race == eldervamprace || race == dlc1keeperrace)
			return true;
		const auto alduinfac = RE::TESForm::LookupByID<RE::TESFaction>(0x28347);
		const auto DLC2bendwillimmunefac = RE::TESForm::LookupByID<RE::TESFaction>(0x40200E7);
		const auto jarlfac = RE::TESForm::LookupByID<RE::TESFaction>(0x50920);
		const auto greybeardfac = RE::TESForm::LookupByID<RE::TESFaction>(0x2C6C8);
		if (subject->IsInFaction(alduinfac) || subject->IsInFaction(DLC2bendwillimmunefac) || subject->IsInFaction(jarlfac) || subject->IsInFaction(greybeardfac))
		{
			logger::info("Subject is part of an excluded alliance");
			return true;
		}
		if (subject->IsChild()) {
			logger::warn("Subject is child");
			return true;
		} else {
			std::string str = race->GetFormEditorID();
			std::for_each(str.begin(), str.end(), [](unsigned char c) {
				std::tolower(c); });
			logger::info("Checking if race is child race >> {}", str);
			if (str.find("child") != std::string::npos || str.find("enfant") != std::string::npos || str.find("little") != std::string::npos ||
				str.find("teen") != std::string::npos) {
				logger::warn("Subject is child");
				return true;
			}
		}
		const auto base = subject->GetActorBase();
		if (!base) {
			logger::warn("Actor base is null?");
			return true;
		}
		const auto formid = base->GetFormID();
		// Durnehviir
		if (formid == 0x20030D8) {
			logger::info("Subject is Durnehviir");
			return true;
		}
		const auto vtype = base->GetVoiceType();
		if (!vtype) {
			logger::warn("Voicetype is null?");
			return true;
		}
		auto vid = vtype->GetFormID();
		// Harkon, Tullius, Ulfric, Paarthunax,
		if (vid == 0x2007D86 || vid == 0x1A63A || vid == 0x20C1A || vid == 0x6F451) {
			logger::info("Subject has an excluded voicetype");
			return true;
		}
		// find key for the exclude actor set
		const auto key = createkeypair(formid);
		auto result = excludedactor.contains(key);
		logger::info("Completed validation is excluded = {}", result);
		return result;
	}

	void Configuration::createassault(RE::Actor* primum, std::vector<RE::Actor*> secundi)
	{
		const auto svm = RE::SkyrimVM::GetSingleton();
		auto vm = svm ? svm->impl : nullptr;
		if (vm) {
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			auto args = RE::MakeFunctionArguments(std::move(primum), std::move(secundi));
			vm->DispatchStaticCall("KudasaiAnimation", "CreateAssault", args, callback);
			delete args;
		}
	}

	Configuration::KeyPair Configuration::createkeypair(uint32_t a_id)
	{
		logger::info("Creating KeyPair from ID = {}", a_id);
		uint32_t id;
		uint8_t index0 = a_id >> (4 * 6);
		const auto handler = RE::TESDataHandler::GetSingleton();
		auto file = handler->LookupLoadedModByIndex(index0);
		if (file && file->compileIndex < 0xFF) {
			id = a_id - (index0 << (4 * 6));
			logger::info("Found Plugin >> Index = {} ;; ID = {}", a_id, index0, id);
		} else {
			uint16_t index1 = a_id >> (4 * 5);
			file = handler->LookupLoadedLightModByIndex(index1);
			if (!file || file->compileIndex == 0xFFF)
				return KeyPair(0, "");
			id = a_id - (index1 << (4 * 5));
			logger::info("Found Light Plugin >> Index = {} ;; ID = {}", a_id, index0, id);
		}
		return KeyPair(id, file->GetFilename());
	}

	const bool Configuration::isvalidcreature(RE::Actor* subject)
	{
		logger::info("isvalidcreature on {}", subject->GetFormID());
		auto race = subject->GetRace();
		if (!race)
			return false;
		try {
			std::string strdata = race->GetFormEditorID();
			logger::info("Race ID = {}", strdata);
			const YAML::Node root = YAML::LoadFile("Data\\SKSE\\Plugins\\Kudasai\\Validation.yaml");
			const YAML::Node exc = root["Exceptions"];
			if (exc) {
				logger::info("Number Exceptions = {}", exc.size());
				std::string racestr = race->GetFormEditorID();
				for (auto&& node : exc) {
					if (racestr.find(node.first.as<std::string>()) != std::string::npos) {
						logger::info("Found substring of race ID, node = {}, returning {}", node.first.as<std::string>(), node.second.as<bool>());
						return node.second.as<bool>();
					}
				}
			} else {
				logger::warn("Exception Object not found in file");
			}
			const YAML::Node group = root["RaceGroup"];
			if (!group) {
				logger::warn("Group Object not found in file, returning false");
				return false;
			}
			const auto bpd = race->bodyPartData;
			if (!bpd)
				return false;
			const auto id = bpd->GetFormID();
			const auto res = group[id].IsDefined();
			logger::info("Found Key ( {} ) = {}", id, res);
			return res;
		} catch (const std::exception& e) {
			logger::error(e.what());
			return false;
		}
	}

	const bool Configuration::hasschlong(RE::Actor* subject)
	{
		static const auto Schlongified = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x00AFF8, "Schlongs of Skyrim.esp");
		if (!Schlongified)
			return false;
		return subject->IsInFaction(Schlongified);
	}

	const bool Configuration::isnpc(RE::Actor* subject)
	{
		const auto ActorTypeNPC = RE::TESForm::LookupByID<RE::BGSKeyword>(0x13794);
		return subject->HasKeyword(ActorTypeNPC);
	}

	const bool Configuration::isinterested(RE::Actor* primum, std::initializer_list<RE::Actor*> secundi)
	{
		logger::info("isinterested() on {} and {} aggressors", primum->GetFormID(), secundi.size());
		const auto getgenderkey = [](RE::Actor* actor) {
			auto base = actor->GetActorBase();
			if (base) {
				auto sex = base->GetSex();
				if (sex == RE::SEX::kMale) {
					if (isnpc(actor))
						return Sex::M;
					else
						return Sex::C;
				} else if (sex == RE::SEX::kFemale) {
					if (isnpc(actor)) {
						if (hasschlong(actor))
							return Sex::H;
						else
							return Sex::F;
					} else
						return Sex::Z;
				}
			}
			logger::info("<isactorinterested::getgenderkey> Invalid base or sex");
			return Sex::ERROR;
		};
		const auto keytostring = [](Sex s) {
			switch (s) {
			case Sex::M:
				return "M";
			case Sex::F:
				return "F";
			case Sex::H:
				return "H";
			case Sex::C:
				return "C";
			case Sex::Z:
				return "Z";
			}
			return "?";
		};
		std::string str = keytostring(getgenderkey(primum));
		if (str == "?")
			return false;
		str.append("<-");
		auto sec = std::vector<Sex>();
		for (auto& actor : secundi) {
			auto c = getgenderkey(actor);
			if (c == Sex::ERROR)
				return false;
			sec.push_back(c);
		}
		std::sort(sec.begin(), sec.end());
		std::for_each(sec.begin(), sec.end(), [&](Sex s) {
			str.append(keytostring(s));
		});
		logger::info("String Key = {}", str);
		try {
			const YAML::Node root = YAML::LoadFile("Data\\SKSE\\Plugins\\Kudasai\\Validation.yaml");
			const YAML::Node node = root["Combinations"];
			if (!node) {
				logger::warn("Combinations object not found");
				return false;
			}
			while (str.length() > 3) {
				// check for base key
				if (node[str])
					return node[str].as<bool>();
				// replace with wildcards
				std::string mimic = str;
				for (int i = 3; i < str.length(); i++) {
					for (int n = i; n < str.length(); n++) {
						std::string copy = mimic;
						copy[n] = '*';
						if (node[copy]) {
							logger::info("Found key res = {}", node[copy].as<bool>());
							return node[copy].as<bool>();
						}
					}
					mimic[i] = '*';
				}
				str.pop_back();
			}
			logger::warn("Could not find a valid key for this combination");
			return false;
		} catch (const std::exception& e) {
			logger::error(e.what());
			return false;
		}
	}

}  // namespace Kuasai