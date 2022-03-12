#include "Kudasai/Struggle/Struggly.h"

#include "Kudasai/Animation/Animation.h"
// #include "Kudasai/Interface/QTE.h"

namespace Kudasai::Struggle
{
	Struggly::Struggly(RE::Actor* victim, RE::Actor* aggressor) : //, std::function<void(bool)> callback, std::uint32_t difficulty, StruggleType type) :
		victim(victim), aggressor(aggressor)//, callback(callback), difficulty(difficulty), type(type)
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
			animevents = { vicanim, agranim };

		} catch (const std::exception& e) {
			logger::error(e.what());
			throw InvalidCombination();
		}
	}

	void Struggly::BeginStruggle(RE::Actor* victim, RE::Actor* aggressor) //, std::function<void(bool)> callback, std::uint32_t difficulty, StruggleType type)
	{
		try {
			auto me = new Struggly(victim, aggressor); //, callback, difficulty, type);

			Animation::PlayPaired(victim, aggressor, me->animevents);
			// let the animation lean in before starting the game
			// std::this_thread::sleep_for(std::chrono::seconds(2));
			// // TODO: setup UI Game
			// // TODO: interpret difficulty
			// // TODO: "Input" (mashing) style game
			// // Interface::QTE::callback = std::bind(&Struggly::Result, me, std::placeholders::_1);
			// Interface::QTE::callback = [me](bool victory) {
			// 	if (!victory) {
			// 		logger::info("Player lost the QTE");
			// 		me->callback(victory);
			// 	} else {
			// 		logger::info("Player won QTE");
			// 		Interface::QTE::OpenMenu(); // thatll create an endless loop until player lost :)
			// 	}
			// };
			// Interface::QTE::time = 5;
			// Interface::QTE::OpenMenu();

		} catch (const std::exception& e) {
			logger::error(e.what());
		}
	}
}  // namespace Kudasai::Struggle
