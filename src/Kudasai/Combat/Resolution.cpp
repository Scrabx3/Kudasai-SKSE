#include "Kudasai/Combat/Resolution.h"

#include "Kudasai/Animation/Animation.h"
#include "Papyrus/Settings.h"

namespace Kudasai
{
	Resolution::QuestData::QuestData(const std::string filepath)
	try : filepath(filepath), root(YAML::LoadFile(filepath)) {
		const YAML::Node reqs = root["Requirements"];
		// Required Keys
		quest = [&]() -> RE::TESQuest* {
			const YAML::Node data = root["Data"];
			const auto id = data["FormID"].as<uint32_t>();
			const auto esp = data["ESP_Name"].as<std::string>();
			auto ret = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(id, esp);
			if (!ret)
				throw InvalidConfig(fmt::format("Unable to Locate Quest with ID = {} in {}", id, esp).c_str());

			if (reqs.IsDefined()) {
				if (const auto reqmods = reqs["Mods"]; reqmods.IsDefined()) {
					if (!reqmods.IsSequence()) {
						logger::error("Requirement 'Mods' is defined but not a Sequence");
						return nullptr;
					}
					const auto handler = RE::TESDataHandler::GetSingleton();
					const auto list = reqmods.as<std::vector<std::string>>();
					for (auto& e : list) {
						if (!handler->LookupModByName(e)) {
							logger::info("Mod {} is required not present. The Event will not be loaded", e);
							return nullptr;
						}
					}
				}
			}

			if (auto w = root["Weight"]; w.IsDefined()) {
				if (w.as<int>() == -1)
					w = 0;
			}
			return ret;
		}();
	} catch (const std::exception& e) {
		throw InvalidConfig(fmt::format("{}: {}", filepath, e.what()).c_str());
	}

	const std::string Resolution::QuestData::GetName() const noexcept
	{
		return root["Name"].as<std::string>();
	}

	const int32_t Resolution::QuestData::GetWeight() const
	{
		return root["Weight"].IsDefined() ? root["Weight"].as<int32_t>() : 50;
	}

	void Resolution::QuestData::UpdateWeight(const int32_t value)
	{
		root["Weight"] = value;
	}

	void Resolution::QuestData::WriteFile()
	{
		logger::info("Writing {}", filepath);
		std::ofstream fout(filepath);
		fout << root;
	}

	const bool Resolution::QuestData::CanBlackout() const
	{
		return root["Blackout"].IsDefined() ? root["Blackout"].as<bool>() : []() {
			logger::warn("Blackout Key not defined. Assuming Blackout = false."); return false; }();
	}

	const bool Resolution::QuestData::DoesTP() const
	{
		return root["DoesTeleport"].IsDefined() ? root["DoesTeleport"].as<bool>() : []() {
			logger::warn("DoesTeleport Key not defined. Assuming DoesTeleport = true."); return true; }();
	}

	const bool Resolution::QuestData::MatchesRace(const std::vector<RE::Actor*>& list) const
	{
		const auto reqs = root["Requirements"];
		if (!reqs.IsDefined())
			return true;
		const auto nRaceKey = reqs["RaceKey"];
		if (!nRaceKey.IsDefined())
			return true;
		else if (!nRaceKey.IsSequence()) {
			logger::error("{}: 'RaceKey' Key is defined but not a Sequence", filepath);
			return false;
		}
		auto keys = nRaceKey.as<std::vector<std::string>>();
		const auto all = reqs["RaceKey_All"].IsDefined() ? reqs["RaceKey_All"].as<bool>() : true;
		for (auto& e : list) {
			const auto racekey = Animation::GetRaceKey(e);
			if (racekey.empty())
				continue;
			if (auto it = std::find(keys.begin(), keys.end(), racekey); it != keys.end()) {
				if (all) {
					keys.erase(it);
					if (keys.empty())
						return true;
				} else
					return true;
			}
		}
		return false;
	}

	void Resolution::Register()
	{
		// read through all config files in hostile & friendly, collect QuestData in vectors
		const auto read = [](std::string path, std::vector<QuestData>& list) {
			if (fs::exists(path) == false)
				return;
			for (auto& file : fs::directory_iterator{ path }) {
				try {
					const auto filepath = file.path().string();
					logger::info("Reading File = {}", filepath);
					QuestData data{ filepath };
					list.push_back(data);
				} catch (const std::exception& e) {
					logger::error(e.what());
				}
			}
		};
		read(CONFIGPATH("PostCombat\\Hostile"), Quests.find(Type::Hostile)->second);
		read(CONFIGPATH("PostCombat\\Follower"), Quests.find(Type::Follower)->second);
		read(CONFIGPATH("PostCombat\\Neutral"), Quests.find(Type::Neutral)->second);
		read(CONFIGPATH("PostCombat\\Guard"), Quests.find(Type::Guard)->second);
	}

	void Resolution::UpdateProperties()
	{
		// #c200c2 <- purple for potential blackouts
		// store all hostile quests into the Array in Papyrus to allow manipulation through MCM
		std::vector<std::string> titles{};
		std::vector<int32_t> number{};
		for (auto& data : Quests.find(Type::Hostile)->second) {
			if (titles.size() == 126) {
				logger::warn("Total amount of Consequences is 126, some Consequences will not be listed.");
				break;
			}
			titles.emplace_back(data.CanBlackout() ? fmt::format("<font color = '#c200c2'>{}</font color>", data.GetName()) : data.GetName());
			number.emplace_back(data.quest ? data.GetWeight() : -1);
		}
		Papyrus::SetSetting("ConTitle", titles);
		Papyrus::SetSetting("ConWeight", number);
	}

	void Resolution::UpdateWeights()
	{
		const auto list = Papyrus::GetSetting<std::vector<std::int32_t>>("ConWeight");
		auto it = Quests.find(Type::Hostile)->second.begin();
		for (auto& e : list) {
			it->UpdateWeight(e);
			it->WriteFile();
			it++;
		}
	}

	RE::TESQuest* Resolution::SelectQuest(Type type, const std::vector<RE::Actor*>& list, bool blackout)
	{
		using rType = Resolution::Type;
		bool cantp = Papyrus::Configuration::IsValidTPLoc();
		if (blackout && !cantp)
			return nullptr;

		auto& quests = Quests.find(type)->second;
		if (quests.size()) {
			std::vector<std::pair<RE::TESQuest*, int32_t>> copy{};
			int32_t chambers = 0;
			copy.reserve(quests.size());
			for (auto& e : quests) {
				if (e.quest != nullptr && !e.quest->IsEnabled() && e.MatchesRace(list) && (!blackout || e.CanBlackout()) && (cantp || !e.DoesTP())) {
					const auto w = e.GetWeight();
					if (w <= 0)
						continue;
					chambers += w;
					copy.emplace_back(e.quest, chambers);
				}
			}
			if (!copy.empty()) {
				const auto where = Random::draw<int32_t>(1, chambers);
				const auto there = std::find_if(copy.begin(), copy.end(), [where](std::pair<RE::TESQuest*, int32_t>& pair) { return where <= pair.second; });
				logger::info("<Resolution::SelectQuest> Found Quest: {} (ID = {}) ", there->first->formEditorID, there->first->formID);
				return there->first;
			}
		} else {
			logger::warn("<Resolution::SelectQuest> No Quests");
		}
		logger::warn("<Resolution::SelectQuest> No valid Quests found");
		// Get Default Quest
		switch (type) {
		case rType::Guard:
			return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(QuestGuardDefault, ESPNAME);
		case rType::Hostile:
			return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESQuest>(QuestDefault, ESPNAME);
		// case rType::Follower:
		// case rType::Neutral:
		default:
			return nullptr;
		}
	}

}  // namespace Kudasai::Resolution
