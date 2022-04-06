#include "Settings.h"

#include "Kudasai/Animation/Animation.h"
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
		if (!race)
			return true;
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
			return true;
		if (subject->IsChild())
			return true;
		else {
			std::string_view str = race->GetFormEditorID();
			std::for_each(str.begin(), str.end(), [](unsigned char c) { std::tolower(c); });
			if (str.find("child") != std::string_view::npos || str.find("enfant") != std::string_view::npos || str.find("little") != std::string_view::npos ||
				str.find("teen") != std::string_view::npos)
				return true;
		}
		const auto base = subject->GetActorBase();
		if (!base)
			return true;
		const auto formid = base->GetFormID();
		// Durnehviir
		if (formid == 0x20030D8) {
			logger::info("Subject is Durnehviir");
			return true;
		}
		const auto vtype = base->GetVoiceType();
		if (!vtype)
			return true;
		auto vid = vtype->GetFormID();
		// Harkon, Tullius, Ulfric, Paarthunax,
		if (vid == 0x2007D86 || vid == 0x1A63A || vid == 0x20C1A || vid == 0x6F451) {
			logger::info("Subject has an excluded voicetype");
			return true;
		}
		// find key for the exclude actor set
		return excludedactor.contains(createkeypair(formid));
	}

	void Configuration::createassault(RE::Actor* primum, std::vector<RE::Actor*> secundi)
	{
		const auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto hook = "Kudasai_NativeAssault";
		auto checkarousal = true;
		auto args = RE::MakeFunctionArguments(std::move(primum), std::move(secundi), std::move(hook), std::move(checkarousal));
		RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
		vm->DispatchStaticCall("KudasaiAnimation", "CreateAssault", args, callback);
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
		logger::info("Created KeyPair = {}/{}", id, file->GetFilename());
		return KeyPair(id, file->GetFilename());
	}

	const bool Configuration::isvalidrace(RE::Actor* subject)
	{
		logger::info("isvalidrace on {}", subject->GetFormID());
		auto racekey = Kudasai::Animation::GetRaceKey(subject);
		if (racekey.empty())
			return false;
		else if (racekey == "Human")
			return true;
		
		try {
			const YAML::Node root = YAML::LoadFile(CONFIGPATH("Validation.yaml"));
			const YAML::Node key = root["RaceKeys"][racekey];
			if (key.IsDefined())
				return key.as<bool>();

		} catch (const std::exception& e) {
			logger::error(e.what());
		}
		return false;
	}

	const bool Configuration::hasschlong(RE::Actor* subject)
	{
		const auto schlongified = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x00AFF8, "Schlongs of Skyrim.esp");
		if (!schlongified)
			return false;
		return subject->IsInFaction(schlongified);
	}

	const bool Configuration::isnpc(RE::Actor* subject)
	{
		const auto ActorTypeNPC = RE::TESForm::LookupByID<RE::BGSKeyword>(0x13794);
		return subject->HasKeyword(ActorTypeNPC);
	}

	// TODO: Rewrite this. Take only 1 Aggressor & add a second function check "Can add to Combination"
	const bool Configuration::isinterested(RE::Actor* primum, std::vector<RE::Actor*> secundi)
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
			const YAML::Node root = YAML::LoadFile(CONFIGPATH("Validation.yaml"));
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
