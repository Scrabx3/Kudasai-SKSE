#pragma once

#include "Papyrus/Property.h"

namespace Papyrus
{
	struct Settings :
		public Singleton<Settings>
	{
		void UpdateSettings();

		bool AllowProcessing = true;
		bool AllowConsequence = true;

		bool bEnabled = false;
		bool bCreatureDefeat;

		bool bLethalEssential;
		float fLethalPlayer;
		float fLethalNPC;

		int iStripReq;
		float fStripReqChance;
		float fStripChance;
		float fStripDestroy;
		bool bStripDrop;
	};

	template <class T>
	inline T GetSetting(std::string property)
	{
		const auto form = RE::TESDataHandler::GetSingleton()->LookupForm(Kudasai::QuestMain, ESPNAME);
		const auto mcm = CreateObjectPtr(form, "KudasaiMCM");
		return RE::BSScript::UnpackValue<T>(mcm->GetProperty(property));
	}

	template <class T>
	inline void SetSetting(RE::BSFixedString property, T val)
	{
		const auto form = RE::TESDataHandler::GetSingleton()->LookupForm(Kudasai::QuestMain, ESPNAME);
		auto var = CreateObjectPtr(form, "KudasaiMCM")->GetProperty(property);
		RE::BSScript::PackValue(var, val);
	}

	namespace Configuration
	{
		_NODISCARD const bool IsValidPrerequisite();
		_NODISCARD const bool IsValidTPLoc();
		_NODISCARD const bool IsValidActor(RE::Actor* subject);
		_NODISCARD const bool IsValidRace(RE::Actor* subject);

		_NODISCARD const bool IsStripProtecc(const RE::TESObjectARMO* a_armor);
		_NODISCARD const bool IsInterested(RE::Actor* subject, RE::Actor* partner);
		// _NODISCARD const bool IsGroupAllowed(RE::Actor* subject, std::vector<RE::Actor*> partners);

		_NODISCARD const bool HasSchlong(RE::Actor* subject);
		_NODISCARD const bool IsNPC(RE::Actor* subject);
		_NODISCARD const char GetGender(RE::Actor* subject);

		struct Data :
			public Singleton<Data>
		{
			void LoadData();
			std::vector<RE::FormID> prLCTN;	 // alltime disabled locations
			std::vector<RE::FormID> tpLCTN;	 // tp disabled locations
			std::vector<RE::FormID> exNPC_;	 // excluded actorbases
			std::vector<RE::FormID> exREF_;  // excluded references
			std::vector<RE::FormID> exRACE;  // excluded races

			std::vector<RE::TESFaction*> exFAC_;  // excluded factions
			std::vector<RE::FormID> armKYWD;  // strip destruction procecc

		private:
			RE::FormID IDFromString(const std::string& id)
			{
				const auto split = id.find("|");
				const auto esp = id.substr(split + 1);
				const auto formid = std::stoi(id.substr(0, split), nullptr, 16);
				return RE::TESDataHandler::GetSingleton()->LookupFormID(formid, esp);
			}

			void ReadNode(const std::vector<std::string>& ids, std::vector<RE::FormID>& list)
			{
				for (auto& id : ids) {
					const auto exclude = IDFromString(id);
					if (exclude == 0) {
						logger::info("Cannot exclude = {}, associated file not loaded", id);
					} else {
						list.push_back(exclude);
						logger::info("Excluded Form = {}", id);
					}
				}
			}

			template <class T>
			void ReadNode(const std::vector<std::string>& ids, std::vector<T*>& list)
			{
				for (auto& id : ids) {
					const auto exclude = IDFromString(id);
					if (exclude == 0) {
						logger::info("Cannot exclude = {}, associated file not loaded", id);
					} else {
						list.push_back(RE::TESForm::LookupByID<T>(exclude));
						logger::info("Excluded Form = {}", id);
					}
				}
			}

		};
	};	// class Configuration

}  // namespace Papyrus