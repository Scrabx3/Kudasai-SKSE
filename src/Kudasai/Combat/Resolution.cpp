#include "Kudasai/Combat/Resolution.h"

#include "Kudasai/Animation/Animation.h"
#include "Papyrus/Settings.h"

namespace Kudasai::Resolution
{
	QuestData::QuestData(const std::string filepath)
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
			return ret;
		}();
	} catch (const std::exception& e) {
		throw InvalidConfig(fmt::format("{}: {}", filepath, e.what()).c_str());
	}

	const std::string QuestData::GetName() const noexcept
	{
		return root["Name"].as<std::string>();
	}

	const int32_t QuestData::GetWeight()
	{
		return root["Weight"].IsDefined() ? root["Weight"].as<int32_t>() : [&]() {
			UpdateWeight(50); return 50; }();
	}

	void QuestData::UpdateWeight(const int32_t value)
	{
		root["Weight"] = value;
	}

	void QuestData::WriteFile()
	{
		std::ofstream fout(filepath);
		fout << root;
	}

	const bool QuestData::CanBlackout() const
	{
		return root["Blackout"].IsDefined() ? root["Blackout"].as<bool>() : []() {
			logger::warn("Blackout Key not defined. Assuming Blackout = false."); return false; }();
	}

	const bool QuestData::MatchesRace(std::vector<RE::Actor*> list) const
	{
		if (!root["RaceKey"].IsDefined())
			return true;
		else if (!root["RaceKey"].IsSequence()) {
			logger::error("{}: 'RaceKey' Key is defined but not a Sequence", filepath);
			return false;
		}
		auto keys = root["RaceKey"].as<std::vector<std::string>>();
		const auto all = root["RaceKey_All"].IsDefined() ? root["RaceKey_All"].as<bool>() : true;
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

	void Register()
	{
		// read through all config files in hostile & friendly, collect QuestData in vectors
		const auto read = [](std::string path, std::vector<QuestData>& list) {
			if (!fs::exists{ path })
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
		read(CONFIGPATH("PostCombat\\Hostile"), HostileQuests);
		read(CONFIGPATH("PostCombat\\Friendly"), FriendlyQuests);
	}

	void UpdateProperties()
	{
		// #c200c2 <- purple for potential blackouts
		// store all hostile quests into the Array in Papyrus to allow manipulation through MCM
		std::vector<std::string> titles{};
		std::vector<int32_t> number{};
		for (auto& data : HostileQuests) {
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

	void UpdateWeights()
	{
		const auto list = Papyrus::GetSetting<std::vector<std::int32_t>>("ConWeight");
		auto it = HostileQuests.begin();
		for (auto& e : list) {
			it->UpdateWeight(e);
			it++;
		}
	}

	void WriteFiles()
	{
		for (auto& e : HostileQuests) {
			e.WriteFile();
		}
	}

	RE::TESQuest* GetQuestHostile(std::vector<RE::Actor*> list, const bool blackout)
	{
		return SelectQuest(HostileQuests, list, blackout);
	}

	RE::TESQuest* GetQuestFriendly(std::vector<RE::Actor*> list)
	{
		return SelectQuest(FriendlyQuests, list, false);
	}

	RE::TESQuest* SelectQuest(std::vector<QuestData>& quests, std::vector<RE::Actor*>& list, const bool blackout)
	{
		if (!quests.size())
			return nullptr;

		std::vector<std::pair<RE::TESQuest*, int32_t>> copy{};
		int32_t chambers = 0;
		copy.reserve(quests.size());
		for (auto& e : quests) {
			if (e.quest != nullptr && e.MatchesRace(list) && (!blackout || e.CanBlackout())) {
				const auto w = e.GetWeight();
				chambers += w;
				copy.emplace_back(e.quest, chambers);
			}
		}
		if (copy.empty())
			return nullptr;

		const int32_t where = randomINT<int32_t>(1, chambers);
		const auto there = std::find_if(copy.begin(), copy.end(), [where](std::pair<RE::TESQuest*, int32_t>& pair) { return where <= pair.second; });
		return there->first;
	}

}  // namespace Kudasai::Resolution
