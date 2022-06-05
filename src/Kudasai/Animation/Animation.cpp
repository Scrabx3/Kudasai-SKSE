#include "Kudasai/Animation/Animation.h"

#include "Papyrus/Settings.h"

namespace Kudasai::Animation
{
	const std::string Animation::GetRaceKey(RE::Actor* subject)
	{
		logger::info("Getting RaceKey for Actor = {}", subject->GetFormID());
		if (Papyrus::Configuration::IsNPC(subject))
			return "Human"s;

		auto race = subject->GetRace();
		if (!race)
			return ""s;

		switch (race->GetFormID()) {
		case 0x0401B658:  // Ash Hopper
			return "AshHopper"s;
		case 0x000131F5:  // Atronach Flame
			return "AtronachFlame"s;
		case 0x000131F6:  // Atronach Frost
			return "AtronachFrost"s;
		case 0x000131F7:  // Atronach Storm
		case 0x04027BFC:  // Ash Guardian
			return "AtronachStorm"s;
		case 0x000131E8:  // Cave Bear
		case 0x000131E7:  // Brown Bear
		case 0x000131E9:  // Snow Bear
			return "Bear"s;
		case 0x04024038:  // Boar
			return "Boar"s;
		case 0x000131EB:  // Chaurus
		case 0x02015136:  // Frozen Chaurus
			return "Chaurus"s;
		case 0x000A5601:  // Chaurus Reapeer
			return "ChaurusReapeer"s;
		case 0x020051FB:  // Chaurus Hunter
			return "ChaurusHunter"s;
		case 0x000A919D:  // Chicken
			return "Chicken"s;
		case 0x0004E785:  // Cow
			return "Cow"s;
		case 0x000CF89B:  // Deer
		case 0x000131ED:  // Elk
		case 0x0200D0B2:  // DLC1 Vale Deer
			return "Deer"s;
		case 0x00012E82:  // Dragon
		case 0x001052A3:  // Dragon Undead
			return "Dragon"s;
		case 0x000131EF:  // Dragon Priest
		case 0x000EBE18:  // Necro Dragon Priest
		case 0x0403911A:  // DLC2 Acolyte Dragon Priest
			return "DragonPriest"s;
		case 0x0402A6FD:  // DLC2 Hulking Draugr
		case 0x0403CECB:  // DLC2 Rigid Skeleton
		case 0x0401B637:  // DLC2 Ash Spawn
		case 0x0200894D:  // DLC1 Soul Cairn Armored Skeleton
		case 0x020023E2:  // DLC1 Armored Skeleton
		case 0x02019FD3:  // DLC1 Black Skeleton Skeleton
		case 0x0200A94B:  // DLC1 Soul Cairn Boneman
		case 0x02006AFA:  // DLC1 Necro Skeleton
		case 0x000B7998:  // Skeleton
		case 0x000B9FD7:  // Rigid Skeleton
		case 0x000EB872:  // Necro Skeleton
		case 0x00000D53:  // Draugr
		case 0x000F71DC:  // Draugr Magic
			return "Draugr"s;
		case 0x000131F1:  // DwarvenCenturion
		case 0x02015C34:  // DLC1 Forgemaster
			return "DwarvenCenturion"s;
		case 0x0402B014:  // DwarvenBallista
			return "DwarvenBallista"s;
		case 0x000131F2:  // DwarvenSphere
			return "DwarvenSphere"s;
		case 0x000131F3:  // DwarvenSpider
			return "DwarvenSpider"s;
		case 0x000131F4:  // Falmer
		case 0x0201AACC:  // DLC1 Frozen Falmer
			return "Falmer"s;
		case 0x000131F8:  // FrostbiteSpider
			return "FrostbiteSpider"s;
		case 0x00053477:  // FrostbideSpiderLarge
			return "FrostbideSpiderLarge"s;
		case 0x0004E507:  // FrostbiteSpiderGiant
			return "FrostbiteSpiderGiant"s;
		case 0x0200A2C6:  // Gargoyle
		case 0x02010D00:  // Gargoyle Variant Boss
		case 0x02019D86:  // Gargoyle Variant Green
			return "Gargoyle"s;
		case 0x0401CAD8:  // DLC2 Frost Giant
		case 0x04014495:  // DLC2 Lurker
		case 0x000131F9:  // Giant
			return "Giant"s;
		case 0x0006FC4A:  // Domestic Goat
		case 0x000131FA:  // Goat
			return "Goat"s;
		case 0x000131FB:  // Hagraven
			return "Hagraven"s;
		case 0x0006DC99:  // Hare
			return "Hare"s;
		case 0x000131FC:  // Horker
			return "Horker"s;
		case 0x000131FD:  // Horse
			return "Horse"s;
		case 0x0401F98F:  // DLC2 Spectral
		case 0x04029EE7:  // DLC2 Karstaag IceWrath
		case 0x000131FE:  // IceWrath
			return "IceWrath"s;
		case 0x000131FF:  // Mammoth
			return "Mammoth"s;
		case 0x040179CF:  // MountedRiekling
			return "MountedRiekling"s;
		case 0x0401B647:  // Solstheim Mudcrab
		case 0x000BA545:  // Mudcrab
			return "Mudcrab"s;
		case 0x04028580:  // Netch Calf
		case 0x0401FEB8:  // Netch
			return "Netch"s;
		case 0x0401A50A:  // Thirst Riekling
		case 0x04017F44:  // Riekling
			return "Riekling"s;
		case 0x00013202:  // Snow SabreCat
		case 0x00013200:  // SabreCat
			return "SabreCat"s;
		case 0x0401DCB9:  // Seeker
			return "Seeker"s;
		case 0x000C3EDF:  // White Skeever
		case 0x00013201:  // Skeever
			return "Skeever"s;
		case 0x00013203:  // Slaughterfish
			return "Slaughterfish"s;
		case 0x0401B644:  // Burned Spriggan
		case 0x02013B77:  // Spriggan Earth Mother
		case 0x000F3903:  // Spriggan Matron
		case 0x00013204:  // Spriggan
			return "Spriggan"s;
		case 0x020117F4:  // Armored Frost Troll
		case 0x02011F75:  // Armored Troll
		case 0x00013206:  // Frost Troll
		case 0x00013205:  // Troll
			return "Troll"s;
		case 0x0200283A:  // VampireBeast (Vampire Lord)
			return "VampireBeast"s;
		case 0x0401E17B:  // Werebear
		case 0x000CDD84:  // Werewolf
			return "Werewolf"s;
		case 0x02003D02:  // Death Hound Companion
		case 0x0200C5F0:  // Death Hound
		case 0x02003D01:  // DLC1 Armored Husky Companion
		case 0x02018B33:  // DLC1 Armored Husky
		case 0x020122B7:  // DLC1 Husky Bare Companion
		case 0x02018B36:  // DLC1 Husky Bare
		case 0x000CD657:  // Barbas Race
		case 0x000F1AC4:  // Dog Companion
		case 0x000131EE:  // Dog
		case 0x000F905F:  // Dog (MG07)
		case 0x00109C7C:  // Fox
		case 0x0001320A:  // Wolf
			return "Wolf"s;
		}
		return ""s;
	}

	std::vector<std::string> LookupStruggleAnimations(std::vector<RE::Actor*> positions)
	{
		if (!Papyrus::Configuration::IsNPC(positions[0]))
			throw InvalidAnimationRequest();

		const std::string racekey{ Animation::GetRaceKey(positions[1]) };
		if (racekey.empty())
			throw InvalidAnimationRequest();

		std::vector<std::string> anims{};
		anims.reserve(positions.size());
		try {
			YAML::Node root = YAML::LoadFile(CONFIGPATH("Struggle.yaml"));
			YAML::Node node = root[racekey];
			if (!node.IsDefined() || !node.IsMap())
				throw InvalidAnimationRequest();

			// IDEA: consider rotation of the two Actors to play different animations? Baka might do something for this
			// 3p+ support? multiple struggle sets?
			anims.emplace_back(node["Victim"].as<std::string>());
			anims.emplace_back(node["Aggressor"].as<std::string>());
			if (std::find(anims.begin(), anims.end(), ""s) != anims.end())
				throw InvalidAnimationRequest();

		} catch (const std::exception& e) {
			logger::error(e.what());
			throw InvalidAnimationRequest();
		}
		return anims;
	}

	std::vector<std::string> LookupBreakfreeAnimations(std::vector<RE::Actor*> positions) noexcept
	{
		const std::string racekey{ Animation::GetRaceKey(positions[1]) };
		YAML::Node root = YAML::LoadFile(CONFIGPATH("Struggle.yaml"));
		YAML::Node node = root[racekey]["Breakfree"];
		if (node.IsDefined() && node.IsMap()) {
			auto vicanim = node["Victim"].as<std::string>();
			auto agranim = node["Aggressor"].as<std::string>();
			return { vicanim, agranim };
		} else {
			logger::info("No Struggle for Racekey = {}, falling back to default", racekey);
			ConsolePrint("[Kudasai] Struggle has no Outro. Rooting to default");
			return { "IdleForceDefaultState"s, "StaggerStart"s };
		}
	}

	std::vector<std::string> LookupKnockoutAnimations(std::vector<RE::Actor*>) noexcept
	{
		return { "StaggerStart"s, "StaggerStart"s };
	}

	void SetPositions(const std::vector<RE::Actor*> positions)
	{
		const auto rootobj = RE::TESDataHandler::GetSingleton()->LookupForm(StaticAnimationRoot, ESPNAME);
		const auto plwhere = std::find_if(positions.begin(), positions.end(), [](RE::Actor* subject) { return subject->IsPlayerRef(); });
		const auto centeractor = plwhere == positions.end() ? positions[0] : *plwhere;

		const auto center = PlaceAtMe(centeractor, rootobj);
		const auto centerPos = center->GetPosition();
		const auto centerAng = center->GetAngle();

		for (auto&& subject : positions) {
			SetRestrained(subject, true);
			StopTranslating(subject);
			SetVehicle(subject, center);
			subject->data.angle = centerAng;
			subject->SetPosition(centerPos, true);
			subject->Update3DPosition(true);
		}

		const auto setposition = [centerAng, centerPos](RE::Actor* actor) {
			for (size_t i = 0; i < 6; i++) {
				std::this_thread::sleep_for(std::chrono::milliseconds(300));
				actor->data.angle.z = centerAng.z;
				actor->SetPosition(centerPos, false);
			}
		};
		std::for_each(positions.begin(), positions.end(), [&setposition](RE::Actor* subject) { std::thread(setposition, subject).detach(); });
	}

	void ClearPositions(const std::vector<RE::Actor*> positions)
	{
		for (auto& subject : positions) {
			SetRestrained(subject, false);
			SetVehicle(subject, nullptr);
			subject->Update3DPosition(true);
		}
	}

	void PlayAnimation(RE::Actor* subject, const char* animation)
	{
		SKSE::GetTaskInterface()->AddTask([=]() {
			subject->NotifyAnimationGraph(animation);
		});
	}
}  // namespace Kudasai
