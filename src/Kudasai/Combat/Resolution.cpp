#include "Kudasai/Combat/Resolution.h"

#include "Kudasai/Animation/Animation.h"
#include "Papyrus/Settings.h"

namespace Kudasai
{
	Resolution::QuestData::QuestData(const std::string filepath)
	try : filepath(filepath), root(YAML::LoadFile(filepath)) {
		quest = [&]() -> RE::TESQuest* {
			const YAML::Node data = root["Data"];
			const auto id = data["FormID"].as<uint32_t>();
			const auto esp = data["ESP_Name"].as<std::string>();
			auto ret = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(id, esp);
			if (!ret)
				throw InvalidConfig(fmt::format("Unable to Locate Quest with ID = {} in {}", id, esp).c_str());

			if (const YAML::Node reqs = root["Requirements"]; reqs.IsDefined()) {
				if (const auto mods = reqs["Mods"]; mods.IsDefined()) {
					if (!mods.IsSequence())
						throw InvalidConfig("Attribute \"Masters\" is of invalid type");

					const auto handler = RE::TESDataHandler::GetSingleton();
					const auto list = mods.as<std::vector<std::string>>();
					for (auto& e : list) {
						if (!handler->LookupModByName(e)) {
							logger::info("Mod {} is required but not present. The Event will not be loaded", e);
							return nullptr;
						}
					}
				}
			}
			return ret;
		}();
	} catch (const std::exception& e) {
		throw InvalidConfig(e.what());
	}

	int Resolution::QuestData::ValidateRequirements(const std::vector<RE::Actor*>& list) const
	{
		/*
		  Priority: 0 # between 0 and 10, if two or more events can be started, the one with higher Priority is used
			Masters:    # Required Masters for this Event, to create dependency on internal soft requirements
				- Master.esp

			# Requirements put on the winning party of the encounter
			RaceType:     # ALL
				- Human
				- Wolf
			ActorBase:    # ANY
				- Mod.esp|0x3456
				- Wood.esp|0xABC
			Reference:    # ANY
				- Mod.esp|0x4C70
			Faction:      # ALL
				- Skyrim.esm|0x2BF9B      # Stormcloaks

			# Requirements put on the player specifically
			PlayerInFaction:      # ALL
				- Skyrim.esm|0x2BF9A      # Imperial Soldiers
			PlayerHasMagicEffect: # ALL
				- Mod.esp|0x746
			
			# Requirements on story progress
			QuestCompleted:   # ANY
				- Skyrim.esm|0x3372B      # MQ101 "Unbound"
			QuestRunning:     # ANY
				- Mod.esp|0x1D6AF
		 */

		try {
			const YAML::Node reqs = root["Requirements"];
			if (!reqs.IsDefined())
				return 0;
			const auto priority = reqs["Priority"].IsDefined() ? std::max(0, std::min(reqs["Priority"].as<int>(), 10)) : 0;
			// base checks
			if (const auto quests = ReadFormsFromYaml<RE::TESQuest>(reqs, "QuestCompleted"); !quests.empty()) {
				for (auto& q : quests)
					if (!q->IsCompleted())
						return -1;
			}
			if (const auto quests = ReadFormsFromYaml<RE::TESQuest>(reqs, "QuestRunning"); !quests.empty()) {
				for (auto& q : quests)
					if (!q->IsRunning())
						return -1;
			}
			// player centric
			const auto player = RE::PlayerCharacter::GetSingleton();
			if (const auto factions = ReadFormsFromYaml<RE::TESFaction>(reqs, "PlayerInFaction"); !factions.empty()) {
				for (auto& f : factions)
					if (!player->IsInFaction(f))
						return -1;
			}
			if (const auto effects = ReadFormsFromYaml<RE::EffectSetting>(reqs, "PlayerHasMagicEffect"); !effects.empty()) {
				for (auto& e : effects)
					if (!player->HasMagicEffect(e))
						return -1;
			}
			// list specific
			std::vector<std::string> racetypes = reqs["RaceType"].IsDefined() ? reqs["RaceType"].as<decltype(racetypes)>() : decltype(racetypes){};
			auto factions = ReadFormsFromYaml<RE::TESFaction>(reqs, "Faction");
			auto bases = ReadFormIDsFromYaml(reqs, "ActorBase");
			auto refs = ReadFormIDsFromYaml(reqs, "Reference");
			for (auto& npc : list) {
				for (auto i = factions.end() - 1; i > factions.begin(); i--) {
					if (npc->IsInFaction(*i)) {
						factions.erase(i);
					}
				}
				auto racetype = Animation::GetRaceType(npc);
				for (auto i = racetypes.end() - 1; i > racetypes.begin(); i--) {
					if (racetype == *i) {
						racetypes.erase(i);
						break;
					}
				}
				if (std::find(bases.begin(), bases.end(), npc->GetTemplateActorBase()->GetFormID()) != bases.end()) {
					bases.clear();
				}
				if (std::find(refs.begin(), refs.end(), npc->GetFormID()) != refs.end()) {
					refs.clear();
				}
			}
			return racetypes.empty() && factions.empty() && bases.empty() && refs.empty() ? priority : -1;
		} catch (const std::exception& e) {
			logger::error(e.what());
			return false;
		}
	}

	void Resolution::Register()
	{
		// read through all config files in hostile & friendly, collect QuestData in vectors
		const auto read = [](std::string path, std::vector<QuestData>& list) {
			if (!fs::exists(path))
				return;
			for (auto& file : fs::directory_iterator{ path }) {
				try {
					const auto filepath = file.path().string();
					logger::info("Reading File = {}", filepath);
					const auto& data = list.emplace_back(filepath);
					logger::info("Successfully added Event = {}", data.GetName());
				} catch (const std::exception& e) {
					logger::error(e.what());
				}
			}
		};
		read(CONFIGPATH("PostCombat\\Hostile"), Quests[Type::Hostile]);
		read(CONFIGPATH("PostCombat\\Follower"), Quests[Type::Follower]);
		read(CONFIGPATH("PostCombat\\Civilian"), Quests[Type::Civilian]);
		read(CONFIGPATH("PostCombat\\Guard"), Quests[Type::Guard]);
	}

	void Resolution::UpdateProperties()
	{
		// store all hostile quests into the Array in Papyrus to allow manipulation through MCM
		std::vector<std::string> titles{}, descriptions{};
		std::vector<int32_t> number{};
		for (auto& data : Quests[Type::Hostile]) {
			if (data.IsHidden()) {
				continue;
			} else if (titles.size() == 126) {
				logger::warn("Can only display 126 events. Some consequences will not be listed.");
				break;
			}
			titles.emplace_back(data.IsBlackout() ? fmt::format("*{}", data.GetName()) : data.GetName());
			descriptions.emplace_back(data.GetDescription());
			number.emplace_back(data.quest ? data.GetWeight() : -1);
		}
		Papyrus::SetSetting("ConTitle", titles);
		Papyrus::SetSetting("ConWeight", number);
		Papyrus::SetSetting("ConDescription", descriptions);
	}

	void Resolution::UpdateWeights()
	{
		const auto list = Papyrus::GetSetting<std::vector<std::int32_t>>("ConWeight");
		auto it = Quests[Type::Hostile].begin();
		for (auto& e : list) {
			it->UpdateWeight(e);
			it->WriteFile();
			it++;
		}
	}

	RE::TESQuest* Resolution::SelectQuest(Type type, const std::vector<RE::Actor*>& list, bool blackout)
	{
		if (Quests[type].empty()) {
			logger::info("<Resolution::SelectQuest> No quests for Type {}", type);
		} else {
			bool allowteleport = Papyrus::Configuration::IsValidTPLoc();
			int maxprio = 0;
			int weights = 0;
			std::vector<std::pair<RE::TESQuest*, decltype(weights)>> quests{};
			for (auto& e : Quests[type]) {
				if (!e.quest || blackout && !e.IsBlackout() || !allowteleport && e.IsTeleport())
					continue;
				const auto prio = e.ValidateRequirements(list);
				if (prio < maxprio) {
					continue;
				}
				const auto w = e.GetWeight();
				if (w <= 0) {
					continue;
				} else if (prio > maxprio) {
					quests.clear();
					maxprio = prio;
					weights = 0;
				}
				weights += w;
				quests.emplace_back(e.quest, weights);
			}
			if (!quests.empty()) {
				const auto where = Random::draw<decltype(weights)>(1, weights);
				const auto there = std::find_if(quests.begin(), quests.end(), [where](std::pair<RE::TESQuest*, int32_t>& pair) { return where <= pair.second; });
				logger::info("<Resolution::SelectQuest> Selecting Quest: {} / {}) ", there->first->GetFormEditorID(), there->first->GetFormID());
				return there->first;
			}
		}
		logger::warn("<Resolution::SelectQuest> Unable to select Quest. Using Default");
		switch (type) {
		case Type::Guard:
			return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(QuestGuardDefault, ESPNAME);
		case Type::Hostile:
			return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(QuestDefault, ESPNAME);
		default:
			return nullptr;
		}
	}

}  // namespace Kudasai::Resolution
