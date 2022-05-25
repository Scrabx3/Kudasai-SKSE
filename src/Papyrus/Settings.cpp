#include "Settings.h"

#include "Kudasai/Animation/Animation.h"
namespace Papyrus::Configuration
{
	const bool IsValidActor(RE::Actor* subject)
	{
		if (subject->IsPlayerRef())
			return true;
		else if (subject->IsChild())
			return false;

		const auto base = subject->GetActorBase();
		if (!base)
			return false;
		const auto formid = base->GetFormID();
		switch (formid) {
		case 0x0001327E:  // Tulius
		case 0x0001414D:  // Ulfric
			{
				const auto CWSiege = RE::TESForm::LookupByID<RE::TESQuest>(0x00096E71);
				if (CWSiege->IsEnabled()) {
					for (const auto& objective : CWSiege->objectives) {
						// "Bring Ulfric/Tullius to justice" objectives
						if (objective->state.all(RE::QUEST_OBJECTIVE_STATE::kDisplayed) && objective->index > 4000 && objective->index <= 4102)
							return false;
					}
				}
			}
			break;
		case 0x00013387:  // Anton
		case 0x00038C6E:  // BalagogGroNolob
			{
				const auto DB08 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA57);
				if (DB08->currentStage == 4 || DB08->currentStage == 6)
					return false;
			}
			break;
		case 0x0001BDB1:  // Cicero
			{
				const auto DB07 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA56);
				if (DB07->currentStage >= 40 && DB07->currentStage < 200)
					return false;
			}
			break;
		case 0x0001327A:  // Vittoria Vici
			{
				const auto DB05 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA54);
				if (DB05->IsEnabled())
					return false;
			}
			break;
		case 0x0001B074:  // Alain Dufont
		case 0x0001412C:  // Nilsine Shatter-Shield
			{
				const auto DB03 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA53);
				if (DB03->IsEnabled())
					return false;
			}
			break;
		case 0x0001B07C:	// Mercer Frey
			{
				const auto TG08b = RE::TESForm::LookupByID<RE::TESQuest>(0x00021554);
				if (TG08b->IsEnabled())
					return false;
			}
			break;
		case 0x0001A694:  // Vilkas
			{
				const auto& C00VilkasTrainingQuest = RE::TESForm::LookupByID<RE::TESQuest>(0x000A3EBC);
				if (C00VilkasTrainingQuest->IsEnabled())
					return false;
			}
			break;
		case 0x02003368:  // Stalf
		case 0x02003369:  // Salonia Caelia
			{
				const auto& DLC1VampIntro = RE::TESForm::LookupByID<RE::TESQuest>(0x0200594C);
				if (DLC1VampIntro->currentStage == 40)
					return false;
			}
			break;
		case 0x20058B0:	 // Dexion
			{
				const auto DLC1VQ3Hunter = RE::TESForm::LookupByID<RE::TESQuest>(0x020098CB);
				const auto DLC1VQ03Vamp = RE::TESForm::LookupByID<RE::TESQuest>(0x020098CB);
				if (DLC1VQ3Hunter->IsEnabled() || DLC1VQ03Vamp->IsEnabled())
					return false;
			}
			break;
		case 0x00013268:  // Deeja
		case 0x0001328D:  // Jaree-Ra
			{
				const auto& LightsOut = RE::TESForm::LookupByID<RE::TESQuest>(0x00023A64);
				if (LightsOut->currentStage > 100)
					return false;
			}
			break;
		case 0x0001414A:  // Calixto
			{
				const auto& MS11 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001F7A3);
				if (MS11->IsEnabled())
					return false;
			}
			break;
		default:
			if (Data::GetSingleton()->exNPC_.contains(formid))
				return false;
			break;
		}

		const auto furnihandle = subject->GetOccupiedFurniture();
		if (const auto furni = furnihandle.get(); furni) {
			const auto DA02BoethiahPillar = RE::TESForm::LookupByID<RE::BGSKeyword>(0x000F3B64);
			if (furni->HasKeyword(DA02BoethiahPillar))
				return false;
		}

		const auto race = subject->GetRace();
		if (!race || race == RE::TESForm::LookupByID<RE::TESRace>(0x2007AF3))  // dlc1 keeper
			return false;
		else {
			std::string_view str = race->GetFormEditorID();
			std::for_each(str.begin(), str.end(), [](unsigned char c) { std::tolower(c); });
			if (str.find("child") != std::string_view::npos || str.find("enfant") != std::string_view::npos || str.find("little") != std::string_view::npos ||
				str.find("teen") != std::string_view::npos || str.find("elder") != std::string_view::npos)
				return false;
		}
		if (subject->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x28347)) ||	 // alduin faction
			subject->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x40200E7)) ||	 // dlc2 bend will immune (miraak + minions)
			subject->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x0050920)) ||	 // Jarl
			subject->IsInFaction(RE::TESForm::LookupByID<RE::TESFaction>(0x02C6C8)))	 // greybeards
			return false;

		return true;
	}

	const bool IsValidPrerequisite()
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		if (const auto loc = player->GetCurrentLocation(); loc) {
			if (Data::GetSingleton()->prLCTN.contains(loc->formID))
				return false;
		}

		if (const auto& DGIntimidateQuest = RE::TESForm::LookupByID<RE::TESQuest>(0x00047AE6); DGIntimidateQuest->IsEnabled())	// Brawl Quest
			return false;
		else if (const auto& DLCVQ08 = RE::TESForm::LookupByID<RE::TESQuest>(0x02007C25); DLCVQ08->currentStage == 60)	// Harkon
				return false;
		else if (const auto& DLC1VQ07 = RE::TESForm::LookupByID<RE::TESQuest>(0x02002853); DLC1VQ07->currentStage == 120)  // Gelebor
			return false;
		else if (const auto& DB02 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA51); DB02->currentStage == 10)  // Abandoned Shack Captives
			return false;
		else if (const auto& DB10 = RE::TESForm::LookupByID<RE::TESQuest>(0x0003CEDA); DB10->currentStage < 200 && DB10->currentStage >= 10)  // Sanctuary Raid
			return false;
		else if (const auto& MG08 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001F258); MG08->currentStage == 30)  // Ancano
			return false;
		else if (const auto& DA01 = RE::TESForm::LookupByID<RE::TESQuest>(0x00028AD6); DA01->currentStage == 85)  // The Black Star, inside the Artifact
			return false;
		return true;
	}

	const bool IsValidTPLoc()
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		if (const auto loc = player->GetCurrentLocation(); loc) {
			if (Data::GetSingleton()->tpLCTN.contains(loc->formID))
				return false;
		}

		if (const auto& MS02 = RE::TESForm::LookupByID<RE::TESQuest>(0x00040A5E); MS02->IsEnabled())  // No1 Escapes Cidhna Mine
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

	void Data::LoadData()
	{
		logger::info("Loading Config Data");
		prLCTN = {
			0x00018C91	// Sovngarde
		};
		exNPC_ = {
			0x0003C57C,	 // Paarthurnax
			0x0200A877,	 // DLC1 Gelebor
			0x020030D8,	 // Durnehviir
			0x0001D4B9,	 // Emperor
			0x0001D4BA,	 // Emperor Decoy
			0x00044050,	 // GaiusMaro (DB06)
			0x00034D97,	 // Estomo (MG07)
			0x000C1908,	 // Red Eagle
			0x000A733B,	 // Vigilant Tyranus (DA10)
			0x000EBE2E,	 // Malkoran's Shade (DA09)
			0x0004D246,	 // The Caller (MG03)
			0x0009C8AA	 // Weylin (MS01)
		};
		tpLCTN = {
			0x00018C91	// Cidhna Mine
		};
		const auto readnode = [](const std::vector<std::string>& ids, std::set<RE::FormID>& list) {
			for (auto& id : ids) {
				const auto split = id.find("|");
				const auto esp = id.substr(split);
				const auto handler = RE::TESDataHandler::GetSingleton();
				if (const auto file = handler->LookupModByName(esp); file) {
					RE::FormID formid = file->compileIndex << (3 * 8);
					formid += file->smallFileCompileIndex << ((1 * 8) + 4);
					const auto res = list.emplace(formid + std::stoi(id.substr(0, split)));
					logger::info("Excluding Form = {}; Success = {}", *res.first, res.second);
				}
			}
		};

		if (fs::exists(CONFIGPATH("Exclusion"))) {
			for (auto& file : fs::directory_iterator{ CONFIGPATH("Exclusion") }) {
				try {
					const auto filepath = file.path().string();
					logger::info("Reading File = {}", filepath);
					const auto root = YAML::LoadFile(filepath);
					// Excluded Actors
					using t = std::vector<std::string>;
					if (const auto node = root["Pre_Location"]; node.IsDefined())
						readnode(node.as<t>(), prLCTN);
					if (const auto node = root["Exl_ActorBase"]; node.IsDefined())
						readnode(node.as<t>(), exNPC_);
					if (const auto node = root["Tp_Location"]; node.IsDefined())
						readnode(node.as<t>(), tpLCTN);
				} catch (const std::exception& e) {
					logger::error(e.what());
				}
			}
		}
		logger::info("Successfully loaded Config Data");
	}
}  // namespace Kuasai
