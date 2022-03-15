#include "Kudasai/Struggle/Struggly.h"

#include "Kudasai/Animation/Animation.h"
#include "Kudasai/Interface/QTE.h"

namespace Kudasai::Struggle
{
	void BeginStruggle(std::function<void(bool)> callback, double difficulty, StruggleType type)
	{
		// TODO: Switch StruggleType
		switch (type)
		{
		case StruggleType::None:
			std::thread([callback, difficulty]() {
				bool victory = randomINT<int>(0, 99) < difficulty;
				std::this_thread::sleep_for(std::chrono::seconds(8));
				callback(victory);
			}).detach();
			break;
		case StruggleType::QTE:
			{
				auto hits = floor((randomINT<int>(1, 100) % 4) - difficulty);
				if (hits == 0)
					hits = randomINT<int>(4, 9);
				else
					hits = abs(hits) + 3;
				
				Interface::QTE::time = difficulty;
				Interface::QTE::handler = [=](bool victory) mutable {
					if (victory) {
						if (--hits > 0) {
							Interface::QTE::time *= randomREAL<double>(0.85, 1.1);
							return true;
						}
					}
					// either won and 0 hits or lost
					callback(victory);
					return false;
				};
				Interface::QTE::OpenMenu();
			}
			break;
		default:
			break;
		}
	}

	void PlayStruggle(RE::Actor* victim, RE::Actor* aggressor)
	{
		if (!Papyrus::Configuration::isnpc(victim))
			throw InvalidCombination();

		const std::string racekey{ Animation::GetRaceKey(aggressor) };
		logger::info("Racekey = {}", racekey);
		if (racekey.empty())
			throw InvalidCombination();

		try {
			YAML::Node root = YAML::LoadFile("Data\\SKSE\\Plugins\\Kudasai\\Struggle.yaml");
			YAML::Node node = root[racekey];
			assert(node.IsDefined() && node.IsMap());

			// IDEA: consider rotation of the two Actors to play different animations? Baka might do something for this
			auto vicanim = node["Victim"].as<std::string>();
			auto agranim = node["Aggressor"].as<std::string>();
			if (vicanim.empty() || agranim.empty())
				throw InvalidCombination();

			std::pair<std::string, std::string> animevents = { vicanim, agranim };
			Animation::PlayPaired(victim, aggressor, animevents);

		} catch (const std::exception& e) {
			logger::error(e.what());
			throw InvalidCombination();
		}
	}

	void PlayBreakfree(RE::Actor* victim, RE::Actor* aggressor) noexcept
	{
		const std::string racekey{ Animation::GetRaceKey(aggressor) };

		YAML::Node root = YAML::LoadFile("Data\\SKSE\\Plugins\\Kudasai\\Struggle.yaml");
		YAML::Node node = root[racekey]["Breakfree"];
		if (node.IsDefined() && node.IsMap()) {
			// IDEA: consider rotation of the two Actors to play different animations? Baka might do something for this
			auto vicanim = node["Victim"].as<std::string>();
			auto agranim = node["Aggressor"].as<std::string>();
			std::pair<std::string, std::string> animevents = { vicanim, agranim };
			Animation::ExitPaired(victim, aggressor, animevents);
		} else {
			logger::info("No Struggle for Racekey = {}, falling back to default", racekey);
			ConsolePrint("[Kudasai] Struggle has no Outro. Rooting to default");
			std::pair<std::string, std::string> animevents = { "IdleForceDefaultState", "IdleForceDefaultState" };
			Animation::ExitPaired(victim, aggressor, animevents);
		}
	}
}  // namespace Kudasai::Struggle
