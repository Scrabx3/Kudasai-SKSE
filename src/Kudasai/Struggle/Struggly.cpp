#include "Kudasai/Struggle/Struggly.h"

#include "Kudasai/Animation/Animation.h"
#include "Kudasai/Interface/QTE.h"

namespace Kudasai
{
	Struggle::Struggle(CallbackFunc callback, RE::Actor* victim, RE::Actor* aggressor) :
		callback(callback), victim(victim), aggressor(aggressor), animations([&]() {
			if (!Papyrus::Configuration::IsNPC(victim))
				throw InvalidCombination();

			const std::string racekey{ Animation::GetRaceKey(aggressor) };
			if (racekey.empty())
				throw InvalidCombination();

			try {
				YAML::Node root = YAML::LoadFile(CONFIGPATH("Struggle.yaml"));
				YAML::Node node = root[racekey];
				if (!node.IsDefined() || !node.IsMap())
					throw InvalidCombination();

				// IDEA: consider rotation of the two Actors to play different animations? Baka might do something for this
				auto vicanim = node["Victim"].as<std::string>();
				auto agranim = node["Aggressor"].as<std::string>();
				if (vicanim.empty() || agranim.empty())
					throw InvalidCombination();

				return std::make_pair(vicanim, agranim);

			} catch (const std::exception& e) {
				logger::error(e.what());
				throw InvalidCombination();
			}
		}())
	{}

	Struggle::~Struggle() noexcept
	{
		logger::info("<Struggle> Deleting Struggle -> Victim = {}, Aggressor = {} -> New total Struggles = {}", victim->GetFormID(), aggressor->GetFormID(), strugglers.size() - 1);
		auto where = std::find(strugglers.begin(), strugglers.end(), this);
		strugglers.erase(where);

		const auto unset = [](RE::Actor* subject) {
			auto Srl = Serialize::GetSingleton();
			if (auto where = Srl->tmpessentials.find(subject->GetFormID()); where != Srl->tmpessentials.end()) {
				subject->boolFlags.reset(RE::Actor::BOOL_FLAGS::kEssential);
				Srl->tmpessentials.erase(where);
			}
		};
		unset(victim);
		unset(aggressor);

		_t.get_id() != std::this_thread::get_id() ? _t.join() : _t.detach();
	}

	void Struggle::BeginStruggle(double difficulty, StruggleType type)
	{
		strugglers.push_back(this);
		logger::info("<Struggle> Beginning New Struggle -> Victim = {}, Aggressor = {} -> New total Struggles = {}", victim->GetFormID(), aggressor->GetFormID(), strugglers.size());
		active = true;

		const auto set = [](RE::Actor* subject) {
			if (subject->boolFlags.none(RE::Actor::BOOL_FLAGS::kEssential)) {
				subject->boolFlags.set(RE::Actor::BOOL_FLAGS::kEssential);
				Serialize::GetSingleton()->tmpessentials.insert(subject->GetFormID());
			}
		};
		set(victim);
		set(aggressor);

		_t = std::thread([=]() {
			Animation::PlayPaired(victim, aggressor, animations);
			// lean in for the animation.. I guess
			std::this_thread::sleep_for(std::chrono::milliseconds(3500));

			switch (type) {
			case StruggleType::None:
				{
					std::this_thread::sleep_for(std::chrono::seconds(8));
					if (active) {
						active = false;
						callback(randomINT<int>(0, 99) < difficulty, this);
					}
				}
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
						if (!active)
							return false;

						if (victory) {
							if (--hits > 0) {
								Interface::QTE::time *= randomREAL<double>(0.85, 1.1);
								return true;
							}
						}
						// either won and 0 hits or lost
						active = false;
						callback(victory, this);
						return false;
					};
					Interface::QTE::OpenMenu();
				}
				break;
			default:
				break;
			}
		});
	}

	void Struggle::PlayBreakfree() noexcept
	{		
		const std::string racekey{ Animation::GetRaceKey(aggressor) };
		YAML::Node root = YAML::LoadFile(CONFIGPATH("Struggle.yaml"));
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
			std::pair<std::string, std::string> animevents = { "IdleForceDefaultState"s, "IdleForceDefaultState"s };
			Animation::ExitPaired(victim, aggressor, animevents);
		}
	}

	void Struggle::StopStruggle(RE::Actor* defeated) noexcept
	{
		if (!active)
			return;

		active = false;
		if (victim->IsPlayerRef()) {
			const auto task = SKSE::GetTaskInterface();
			task->AddUITask([]() {
				Interface::QTE::CloseMenu();
			});
		}
		callback(defeated == aggressor, this);
	}

	Struggle* Struggle::FindPair(RE::Actor* subject)
	{
		for (auto& instance : strugglers)
			if (instance->victim == subject || instance->aggressor == subject)
				return instance;
		return nullptr;
	}

}  // namespace Kudasai
