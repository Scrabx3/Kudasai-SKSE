#include "Settings.h"

#include "Kudasai/Animation/Animation.h"

void Papyrus::Settings::UpdateSettings()
{
	const auto mcm = GetMCM();
	bCreatureDefeat = GetProperty<bool>(mcm, "bCreatureDefeat");

	bLethalEssential = GetProperty<bool>(mcm, "bLethalEssential");
	fLethalPlayer = GetProperty<float>(mcm, "fLethalPlayer");
	fLethalNPC = GetProperty<float>(mcm, "fLethalNPC");

	iStripReq = GetProperty<int32_t>(mcm, "iStripReq");
	fStripReqChance = GetProperty<float>(mcm, "fStripReqChance");
	fStripChance = GetProperty<float>(mcm, "fStripChance");
	fStripDestroy = GetProperty<float>(mcm, "fStripDestroy");
	bStripDrop = GetProperty<bool>(mcm, "bStripDrop");
}

namespace Papyrus::Configuration
{
	bool IsValidActor(RE::Actor* subject)
	{
		static const auto ignored = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(Kudasai::FactionIgnored, ESPNAME);
		if (subject->IsInFaction(ignored))
			return false;
		else if (subject->IsChild())
			return false;
		else if (subject->IsPlayerRef())
			return true;

		const auto data = Data::GetSingleton();
		if (std::binary_search(data->exREF_.begin(), data->exREF_.end(), subject->GetFormID()))
			return false;
		const auto base = Kudasai::GetLeveledActorBase(subject);
		// [&subject]() {
		// 	const auto extra = static_cast<RE::ExtraLeveledCreature*>(subject->extraList.GetByType(RE::ExtraDataType::kLeveledCreature));
		// 	const auto base = extra ? static_cast<RE::TESNPC*>(extra->originalBase) : nullptr;
		// 	return base ? base : subject->GetActorBase();
		// }();
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
		case 0x0001B07C:  // Mercer Frey
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
		case 0x000136C0:  // Narfi
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA5B);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x0001360C:  // Ennodius Papius
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA5E);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00013612:  // Beitild
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA5F);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x0001367B:  // Hern
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA60);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x0001AA63:  // Lubuk
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA61);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00020040:  // Deekus
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA62);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x0001B1D7:  // Ma'randru-jo
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA63);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00013B97:  // Anoriath
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA64);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00020044:  // Agnis
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA65);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00020046:  // Maluril
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA66);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00013657:  // Helvar
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA67);
				if (contract->IsEnabled())
					return false;
			}
			break;
		case 0x00013267:  // Safia
			{
				const auto contract = RE::TESForm::LookupByID<RE::TESQuest>(0x0001EA68);
				if (contract->IsEnabled())
					return false;
			}
			break;
		default:
			{
				const auto& t = data->exNPC_;
				if (std::binary_search(t.begin(), t.end(), formid))
					return false;
			}
			break;
		}

		const auto furnihandle = subject->GetOccupiedFurniture();
		if (const auto furni = furnihandle.get(); furni) {
			const auto DA02BoethiahPillar = RE::TESForm::LookupByID<RE::BGSKeyword>(0x000F3B64);
			if (furni->HasKeyword(DA02BoethiahPillar))
				return false;
		}
		const auto race = subject->GetRace();
		if (std::find(data->exRACE.begin(), data->exRACE.end(), race->GetFormID()) != data->exRACE.end()) 
			return false;
		else {
			std::string str = race->GetFormEditorID();
			std::transform(str.cbegin(), str.cend(), str.begin(), [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
			if (str.find("child") != std::string::npos || str.find("enfant") != std::string::npos || str.find("little") != std::string::npos ||
				str.find("teen") != std::string::npos || str.find("elder") != std::string::npos)
				return false;
		}
		for (auto& f : data->exFAC_)
			if (subject->IsInFaction(f))
				return false;
		return true;
	}

	bool IsValidPrerequisite()
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		if (const auto loc = player->GetCurrentLocation(); loc) {
			const auto& t = Data::GetSingleton()->prLCTN;
			if (std::binary_search(t.begin(), t.end(), loc->formID))
				return false;
		}
		if (static const auto DGIntimidateQuest = RE::TESForm::LookupByID<RE::TESQuest>(0x00047AE6); DGIntimidateQuest->IsEnabled())  // Brawl Quest
			return false;
		else if (static const auto MQ101 = RE::TESForm::LookupByID<RE::TESQuest>(0x0003372B); MQ101->currentStage > 1 && MQ101->currentStage < 1000)  // Vanilla Intro
			return false;
		else if (static const auto DLCVQ08 = RE::TESForm::LookupByID<RE::TESQuest>(0x02007C25); DLCVQ08->currentStage == 60)  // Harkon
			return false;
		else if (static const auto DLC1VQ07 = RE::TESForm::LookupByID<RE::TESQuest>(0x02002853); DLC1VQ07->currentStage == 120)  // Gelebor
			return false;
		else if (static const auto DB10 = RE::TESForm::LookupByID<RE::TESQuest>(0x0003CEDA); DB10->currentStage < 200 && DB10->currentStage >= 10)	 // Sanctuary Raid
			return false;
		else if (static const auto MG08 = RE::TESForm::LookupByID<RE::TESQuest>(0x0001F258); MG08->currentStage == 30)	 // Ancano
			return false;
		else if (static const auto DLC2MQ06 = RE::TESForm::LookupByID<RE::TESQuest>(0x040179D7); DLC2MQ06->currentStage >= 400 && DLC2MQ06->currentStage < 500)  // Miraak
			return false;
		return true;
	}

	bool IsValidTPLoc()
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		if (const auto loc = player->GetCurrentLocation(); loc) {
			const auto& t = Data::GetSingleton()->tpLCTN;
			if (std::binary_search(t.begin(), t.end(), loc->formID))
				return false;
		}
		return true;
	}

	bool IsValidRace(RE::Actor* subject)
	{
		if (Kudasai::IsLight()) {
			return false;
		}
		const auto racekey = Kudasai::Animation::GetRaceType(subject);
		if (racekey.empty())
			return false;
		else if (racekey == "Human")
			return true;

		try {
			const YAML::Node root = YAML::LoadFile(CONFIGPATH("Validation.yaml"));
			const YAML::Node key = root["RaceTypes"][racekey];
			if (key.IsDefined())
				return key.as<bool>();

		} catch (const std::exception& e) {
			logger::error(e.what());
		}
		return false;
	}

	bool IsInterested(RE::Actor* subject, RE::Actor* partner)
	{
		if (Kudasai::IsLight()) {
			return false;
		}
		try {
			const YAML::Node root = YAML::LoadFile(CONFIGPATH("Validation.yaml"))["Selective"];
			std::string key{ GetGender(subject) + "<-" + GetGender(partner) };
			logger::info("Looking for Gender Validation = {} || Follower Victim = {}", key, subject->IsPlayerTeammate());
			if (subject->IsPlayerTeammate())
				if (const auto f1 = root["Follower"]; f1.IsDefined() && f1.IsMap())
					if (const auto f2 = f1[key]; f2.IsDefined())
						return f2.as<bool>();
			const YAML::Node ret = root[key];
			if (ret.IsDefined())
				return ret.as<bool>();

		} catch (const std::exception& e) {
			logger::error(e.what());
		}
		return true;
	}

	bool HasSchlong(RE::Actor* subject)
	{
		const auto schlongified = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x00AFF8, "Schlongs of Skyrim.esp");
		if (!schlongified)
			return false;
		return subject->IsInFaction(schlongified);
	}

	bool Configuration::IsNPC(RE::Actor* subject)
	{
		const auto ActorTypeNPC = RE::TESForm::LookupByID<RE::BGSKeyword>(0x13794);
		return subject->HasKeyword(ActorTypeNPC);
	}

	char GetGender(RE::Actor* subject)
	{
		if (!IsNPC(subject))
			return 'C';
		else if (const auto base = subject->GetActorBase(); base && base->GetSex() == RE::SEX::kFemale)
			return HasSchlong(subject) ? 'H' : 'F';
		else
			return 'M';
	}

	bool IsDaedric(const RE::BGSKeywordForm* a_form)
	{
		return a_form->HasKeywordID(0xA8668);
	}

	bool IsStripProtected(const RE::BGSKeywordForm* a_form)
	{
		const auto& nostrips = Data::GetSingleton()->nostrpKYWD;
		for (auto& keyword : nostrips) {
			if (a_form->HasKeywordID(keyword)) {
				return true;
			}
		}
		return false;
	}

	void Data::LoadData()
	{
		logger::info("Loading Config Data");
		prLCTN = {
			0x00018C91,	 // Sovngarde
			0x0005254C,	 // Abandoned Shack Interior
			0x00018EE6	 // Azuras Star Interior (DA01)
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
			0x0009CB66,	 // Malkoran (DA09)
			0x000EBE2E,	 // Malkoran's Shade (DA09)
			0x0004D246,	 // The Caller (MG03)
			0x0009C8AA,	 // Weylin (MS01)
			0x0009E07A,	 // MQ306DragonA
			0x0009E07B,	 // MQ306DragonB
			0x0009E07C,	 // MQ306DragonC
			0x040285C3,	 // Ebony Warrior
			0x04017F7D,	 // Miraak
			0x04017936,	 // Miraak (MQ01)
			0x04017938,	 // Miraak (MQ02)
			0x04017F81,	 // Miraak (MQ04)
			0x0401FB98,	 // Miraak (MQ06)
			0x040200D9,	 // Miraak (SoulSteal)
			0x04023F7B,	 // MiraakDragon
			0x04031CA5,	 // MiraakDragon (MQ02)
			0x04039B6B,	 // MiraakDragon (MQ06)
			0x0004D6D0	 // Astrid End
		};
		exREF_ = {
			0x000A6F4B,	 // Companion Farkas Werewolf, Ambusher01 (C02)
			0x000A6F1E,	 // Companion Farkas Werewolf, Ambusher02 (C02)
			0x000A6F43,	 // Companion Farkas Werewolf, Ambusher03 (C02)
			0x000A6F0F,	 // Companion Farkas Werewolf, Ambusher04 (C02)
			0x000A6F0E	 // Companion Farkas Werewolf, Ambusher05 (C02)
		};
		tpLCTN = {
			0x00018C91	// Cidhna Mine
		};
		exRACE = {
			0x02007AF3	// DLC1 Keeper
		};
		exFAC_ = {
			RE::TESForm::LookupByID<RE::TESFaction>(0x00028347),  // Alduin fac
			RE::TESForm::LookupByID<RE::TESFaction>(0x00050920),  // Jarl
			RE::TESForm::LookupByID<RE::TESFaction>(0x0002C6C8),  // Greybeards
			RE::TESForm::LookupByID<RE::TESFaction>(0x00103531)	  // Restoration Master Qst
		};
		nostrpKYWD = {
			Kudasai::KeywordNoStrip
		};
		if (fs::exists(CONFIGPATH("Exclusion"))) {
			for (auto& file : fs::directory_iterator{ CONFIGPATH("Exclusion") }) {
				try {
					const auto filepath = file.path().string();
					if (!filepath.ends_with(".yaml") && !filepath.ends_with(".yml"))
						continue;
					logger::info("Reading File = {}", filepath);
					const auto root = YAML::LoadFile(filepath);
					using t = std::vector<std::string>;
					if (const auto node = root["Pre_Location"]; node.IsDefined())
						ReadNode(node.as<t>(), prLCTN);
					if (const auto node = root["Exl_ActorBase"]; node.IsDefined())
						ReadNode(node.as<t>(), exNPC_);
					if (const auto node = root["Tp_Location"]; node.IsDefined())
						ReadNode(node.as<t>(), tpLCTN);
					if (const auto node = root["Exl_Race"]; node.IsDefined())
						ReadNode(node.as<t>(), exRACE);
					if (const auto node = root["Exl_Reference"]; node.IsDefined())
						ReadNode(node.as<t>(), exREF_);
					if (const auto node = root["Exl_Faction"]; node.IsDefined())
						ReadNode<RE::TESFaction>(node.as<t>(), exFAC_);
					if (const auto node = root["ArmorNoStrip"]; node.IsDefined())
						ReadNode(node.as<t>(), nostrpKYWD);
				} catch (const std::exception& e) {
					logger::error(e.what());
				}
			}
		}
		std::sort(prLCTN.begin(), prLCTN.end());
		std::sort(exNPC_.begin(), exNPC_.end());
		std::sort(tpLCTN.begin(), tpLCTN.end());
		std::sort(exRACE.begin(), exRACE.end());
		std::sort(exREF_.begin(), exREF_.end());
		logger::info("Successfully loaded Config Data");
	}
}  // namespace Kuasai
