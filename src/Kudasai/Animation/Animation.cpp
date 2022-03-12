#include "Kudasai/Animation/Animation.h"
#include "Papyrus/Settings.h"

namespace Kudasai::Animation
{
	const std::string Animation::GetRaceKey(RE::Actor* subject)
	{
		logger::info("Getting RaceKey for Actor = {}", subject->GetFormID());
		if (Papyrus::Configuration::isnpc(subject))
			return "Human"s;

		auto race = subject->GetRace();
		if (!race)
			return ""s;

		try {
			auto id = race->GetFormID();
			YAML::Node root = YAML::LoadFile("Data\\SKSE\\Plugins\\Kudasai\\RaceKeys.yaml");
			YAML::Node vanilla = root["Races"];
			assert(vanilla.IsDefined() && vanilla.IsMap());
			YAML::Node key = vanilla[id];
			if (key.IsDefined()) {
				auto res = key.as<std::string>();
				logger::info("Found RaceKey = {}", res);
				return res;
			} else {
				YAML::Node custom = root["AdditionalRaces"];
				assert(custom.IsDefined());
				if (!custom.IsMap())
					logger::info("No custom Keys");
				else {
					logger::info("Looking for custom Key on Race = {]", id);
					for (auto&& node : custom) {
						auto entry = node.first.as<std::string>();
						logger::info("Looking at custom Key = {}", entry);
						auto delimeter = entry.find("->");
						if (delimeter == std::string::npos)
							return ""s;
						auto formid = std::atoi(entry.substr(0, delimeter).c_str());
						auto modname = entry.substr(delimeter + 2);

						auto handler = RE::TESDataHandler::GetSingleton();
						auto form = handler->LookupForm<RE::TESRace>(formid, modname);
						if (form)
							return node.second.as<std::string>();
					}
				}
			}
		} catch (const std::exception& e) {
			logger::error(e.what());
		}
		return ""s;
	}

	RE::TESObjectREFR* const GetRootObject(RE::TESObjectREFR* location)
	{
		logger::info("GetRootObject");
		if (!location)
			return nullptr;
		const auto rootform = []() {
			auto handler = RE::TESDataHandler::GetSingleton();
			auto form = handler->LookupForm(0x803D81, ESPNAME);

			return form;
		}();

		return PlaceAtMe(location, rootform);
	}

	void PlayPaired(RE::Actor* first, RE::Actor* partner, const std::pair<std::string, std::string>& animations)
	{
		logger::info("Playing Paird on {} and {} with Animations = {}, {}", first->GetFormID(), partner->GetFormID(), animations.first, animations.second);
		auto pos = first->GetPosition();
		auto angle = first->GetAngle();
		auto root = GetRootObject(first);
		SetVehicle(first, root);
		SetVehicle(partner, root);
		partner->SetPosition(pos, false);
		partner->data.angle = angle;

		if (first->IsPlayerRef()) {
			SetPlayerAIDriven();
			partner->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kRestrained;
		} else {
			first->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kRestrained;
			if (partner->IsPlayerRef())
				SetPlayerAIDriven();
			else
				partner->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kRestrained;
		}

		first->NotifyAnimationGraph(animations.first);
		partner->NotifyAnimationGraph(animations.second);
	}

	void ClearAnimation(std::initializer_list<RE::Actor*> list)
	{
		for (auto& actor : list) {
			SetVehicle(actor, nullptr);
			if (actor->IsPlayerRef())
				SetPlayerAIDriven(false);
			else
				actor->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kAlive;
		}
	}

}  // namespace Kudasai
