#include "Settings.h"

#include "Kudasai/Animation/Animation.h"
namespace Papyrus::Configuration
{
	const bool IsValidActor(RE::Actor* subject)
	{
		// logger::info("Checking exclusion on subject = {}", subject->GetFormID());
		if (subject->IsPlayerRef())
			return true;

		const auto base = subject->GetActorBase();
		const auto formid = base ? base->GetFormID() : 0x20030D8;
		if (formid == 0x20030D8 ||	// Durnehviir
			Serialize::GetSingleton()->Excluded.contains(formid))
			return false;

		const auto vtype = base->GetVoiceType();
		const auto vid = vtype ? vtype->GetFormID() : 0x2007D86;
		if (vid == 0x2007D86 ||	 // Harkon
			vid == 0x1A63A ||	 // Tullius
			vid == 0x20C1A ||	 // Ulfric
			vid == 0x6F451)		 // Paarthunax
			return false;
		
		const auto race = subject->GetRace();
		if (!race)
			return false;
		else if (race == RE::TESForm::LookupByID<RE::TESRace>(0x67CD8) ||	 // elder
				 race == RE::TESForm::LookupByID<RE::TESRace>(0x00A82BA) ||	 // vampire elder
				 race == RE::TESForm::LookupByID<RE::TESRace>(0x2007AF3))	 // dlc1 keeper
			return false;
		else if (subject->IsChild())
			return false;
		else {
			std::string_view str = race->GetFormEditorID();
			std::for_each(str.begin(), str.end(), [](unsigned char c) { std::tolower(c); });
			if (str.find("child") != std::string_view::npos || str.find("enfant") != std::string_view::npos || str.find("little") != std::string_view::npos ||
				str.find("teen") != std::string_view::npos)
				return false;
		}
		if (subject->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x28347)) ||	 // alduin faction
			subject->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x40200E7)) ||	 // dlc2 bend will immune
			subject->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x0050920)) ||	 // Jarl
			subject->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x02C6C8)))	 // greybeards
			return false;
		return true;
	}

	const bool IsValidRace(RE::Actor* subject)
	{
		logger::info("IsValidRace on {}", subject->GetFormID());
		const auto racekey = Kudasai::Animation::GetRaceKey(subject);
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

	const bool IsInterested([[maybe_unused]] RE::Actor* subject, [[maybe_unused]] RE::Actor* partner)
	{
		// TODO: implement
		return true;

	}
	const bool IsGroupAllowed([[maybe_unused]] RE::Actor* subject, [[maybe_unused]] std::vector<RE::Actor*> partners)
	{
		// TODO: implement... or remove?
		return true;
	}

	const bool HasSchlong(RE::Actor* subject)
	{
		const auto schlongified = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x00AFF8, "Schlongs of Skyrim.esp");
		if (!schlongified)
			return false;
		return subject->IsInFaction(schlongified);
	}

	const bool Configuration::IsNPC(RE::Actor* subject)
	{
		const auto ActorTypeNPC = RE::TESForm::LookupByID<RE::BGSKeyword>(0x13794);
		return subject->HasKeyword(ActorTypeNPC);
	}

}  // namespace Kuasai
