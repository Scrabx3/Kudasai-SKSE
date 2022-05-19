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

		try {
			auto id = race->GetFormID();
			static const YAML::Node root = YAML::LoadFile(CONFIGPATH("RaceKeys.yaml"));
			const YAML::Node vanilla = root["Races"];
			assert(vanilla.IsDefined() && vanilla.IsMap());
			const YAML::Node key = vanilla[id];
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
		const auto rootobj = RE::TESDataHandler::GetSingleton()->LookupForm(0x803D81, ESPNAME);
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


	void PlayPaired(const std::vector<RE::Actor*> positions, const std::vector<std::string> animations)
	{
		const auto task = SKSE::GetTaskInterface();
		task->AddTask([positions, animations]() {
			SetPositions(positions);

			for (auto&& subject : positions) {
				const auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
				auto arg1 = subject;
				auto args = RE::MakeFunctionArguments(std::move(arg1));
				RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
				vm->DispatchStaticCall("KudasaiInternal", "FinalizeAnimationStart", args, callback);

				if (subject->IsInCombat())
					subject->StopCombat();
				if (subject->IsWeaponDrawn())
					SheatheWeapon(subject);
			}
			for (size_t i = 0; i < positions.size(); i++) { positions[i]->NotifyAnimationGraph(animations[i]); }
		});
	}

	void ExitPaired(const std::vector<RE::Actor*> positions, const std::vector<std::string> animations)
	{
		auto task = SKSE::GetTaskInterface();
		task->AddTask([=]() {
			for (size_t i = 0; i < positions.size(); i++) { positions[i]->NotifyAnimationGraph(animations[i]); }
			for (auto& subject : positions) {
				auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
				auto arg1 = subject;
				auto args = RE::MakeFunctionArguments(std::move(arg1));
				RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
				vm->DispatchStaticCall("KudasaiInternal", "FinalizeAnimationEnd", args, callback);

				SetRestrained(subject, false);
				SetVehicle(subject, nullptr);
			}
		});
	}

	void PlayAnimation(RE::Actor* subject, const char* animation)
	{
		SKSE::GetTaskInterface()->AddTask([=]() {
			subject->NotifyAnimationGraph(animation);
		});
	}
}  // namespace Kudasai
