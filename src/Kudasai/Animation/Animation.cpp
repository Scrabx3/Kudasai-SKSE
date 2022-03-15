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
		const auto rootform = []() {
			auto handler = RE::TESDataHandler::GetSingleton();
			auto form = handler->LookupForm(0x803D81, ESPNAME);

			return form;
		}();

		return PlaceAtMe(location, rootform);
	}

	void PlayPaired(RE::Actor* first, RE::Actor* partner, const std::pair<std::string, std::string> animations)
	{
		logger::info("Playing Paird on {} and {} with Animations = {}, {}", first->GetFormID(), partner->GetFormID(), animations.first, animations.second);
		auto task = SKSE::GetTaskInterface();
		task->AddTask([=]() {
			auto pos = first->GetPosition();
			auto angle = first->GetAngle();
			auto root = GetRootObject(first);
			StopTranslating(first);
			StopTranslating(partner);

			SetRestrained(first, true);
			SetRestrained(partner, true);
			SetVehicle(first, root);
			SetVehicle(partner, root);
			partner->SetPosition(pos, true);
			partner->data.angle = angle;

			first->NotifyAnimationGraph(animations.first);
			partner->NotifyAnimationGraph(animations.second);

			// first->data.angle = partner->GetAngle();
		});
	}

	void ExitPaired(RE::Actor* first, RE::Actor* partner, const std::pair<std::string, std::string> animations)
	{
		auto task = SKSE::GetTaskInterface();
		task->AddTask([=]() {
			SetVehicle(first, nullptr);
			SetVehicle(partner, nullptr);

			first->NotifyAnimationGraph(animations.first);
			partner->NotifyAnimationGraph(animations.second);

			SetRestrained(first, false);
			SetRestrained(partner, false);
		});
	}

	void PlayAnimation(RE::Actor* subject, const char* animation)
	{
		auto task = SKSE::GetTaskInterface();
		task->AddTask([=]() {
			subject->NotifyAnimationGraph(animation);
		});
	}

	void ForceDefault(RE::Actor* subject)
	{
		auto task = SKSE::GetTaskInterface();
		task->AddTask([subject]() {
			subject->NotifyAnimationGraph("IdleForceDefaultState");
		});
	}

}  // namespace Kudasai
