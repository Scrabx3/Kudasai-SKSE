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

	void PlayPaired(RE::Actor* first, RE::Actor* partner, const std::pair<std::string, std::string> animations)
	{
		logger::info("Playing Paird on {} and {} with Animations = {}, {}", first->GetFormID(), partner->GetFormID(), animations.first, animations.second);
		auto task = SKSE::GetTaskInterface();
		task->AddTask([=]() {
			const auto rootobj = RE::TESDataHandler::GetSingleton()->LookupForm(0x803D81, ESPNAME);
			const auto centeractor = partner->IsPlayerRef() ? partner : first;

			const auto center = PlaceAtMe(centeractor, rootobj);
			const auto centerPos = center->GetPosition();
			const auto centerAng = center->GetAngle();

			const auto prepare = [&](RE::Actor* subject) {				
				const auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
				auto args = RE::MakeFunctionArguments(std::move(subject));
				RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
				vm->DispatchStaticCall("KudasaiInternal", "FinalizeAnimationStart", args, callback);

				if (subject->IsInCombat())
					subject->StopCombat();
				if (subject->IsWeaponDrawn())
					SheatheWeapon(subject);

				SetRestrained(subject, true);
				StopTranslating(subject);
				SetVehicle(subject, center);
				subject->data.angle = centerAng;
				subject->SetPosition(centerPos, true);
			};
			prepare(first);
			prepare(partner);
			
			first->NotifyAnimationGraph(animations.first);
			partner->NotifyAnimationGraph(animations.second);

			const auto setposition = [centerAng, centerPos](RE::Actor* actor) {
				for (size_t i = 0; i < 6; i++) {
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
					actor->data.angle.z = centerAng.z;
					actor->SetPosition(centerPos, false);
				}
			};
			std::thread(setposition, first).detach();
			std::thread(setposition, partner).detach();
		});
	}

	void ExitPaired(RE::Actor* first, RE::Actor* partner, const std::pair<std::string, std::string> animations)
	{
		auto task = SKSE::GetTaskInterface();
		task->AddTask([=]() {
			const auto clean = [](RE::Actor* subject) {
				auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
				auto args = RE::MakeFunctionArguments(std::move(subject));
				RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
				vm->DispatchStaticCall("KudasaiInternal", "FinalizeAnimationEnd", args, callback);

				SetRestrained(subject, false);
				SetVehicle(subject, nullptr);
			};
			clean(first);
			clean(partner);

			first->NotifyAnimationGraph(animations.first);
			partner->NotifyAnimationGraph(animations.second);
		});
	}

	void PlayAnimation(RE::Actor* subject, const char* animation)
	{
		auto task = SKSE::GetTaskInterface();
		task->AddTask([=]() {
			subject->NotifyAnimationGraph(animation);
		});
	}
}  // namespace Kudasai
