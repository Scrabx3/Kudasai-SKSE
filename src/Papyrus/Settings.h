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
		_NODISCARD bool IsValidPrerequisite();
		_NODISCARD bool IsValidTPLoc();
		_NODISCARD bool IsValidActor(RE::Actor* subject);
		_NODISCARD bool IsValidRace(RE::Actor* subject);

		_NODISCARD bool IsStripProtected(const RE::BGSKeywordForm* a_form);
		_NODISCARD bool IsDaedric(const RE::TESForm* a_form);

		_NODISCARD bool IsInterested(RE::Actor* subject, RE::Actor* partner);

		_NODISCARD bool HasSchlong(RE::Actor* subject);
		_NODISCARD bool IsNPC(RE::Actor* subject);
		_NODISCARD char GetGender(RE::Actor* subject);

		struct Data :
			public Singleton<Data>
		{
			void LoadData();
			std::vector<RE::FormID> prLCTN;		  // alltime disabled locations
			std::vector<RE::FormID> tpLCTN;		  // tp disabled locations
			std::vector<RE::FormID> exNPC_;		  // excluded actorbases
			std::vector<RE::FormID> exREF_;		  // excluded references
			std::vector<RE::FormID> exRACE;		  // excluded races
			std::vector<RE::TESFaction*> exFAC_;  // excluded factions

			std::vector<RE::FormID> nostrpKYWD;	 // strip procection

		private:
			void ReadNode(const std::vector<std::string>& ids, std::vector<RE::FormID>& list)
			{
				for (auto& id : ids) {
					const auto exclude = Kudasai::IDFromString(id);
					if (!exclude) {
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
					const auto exclude = Kudasai::FormFromString<T>(id);
					if (!exclude) {
						logger::info("Cannot exclude = {}, associated file not loaded", id);
					} else {
						list.push_back(exclude);
						logger::info("Excluded Form = {}", id);
					}
				}
			}

		};
	};	// class Configuration

}  // namespace Papyrus