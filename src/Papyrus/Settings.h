#pragma once

#include "Papyrus/Property.h"

namespace Papyrus
{
	static inline bool AllowProcessing{ true };
	static inline bool AllowConsequence{ true };

	inline ObjectPtr GetMCM() { return CreateObjectPtr(RE::TESDataHandler::GetSingleton()->LookupForm(Kudasai::QuestMain, ESPNAME), "KudasaiMCM"); }

	struct Settings :
		public Singleton<Settings>
	{
		void UpdateSettings();

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
	inline T GetSetting(const RE::BSFixedString& a_property)
	{
		return RE::BSScript::UnpackValue<T>(GetMCM()->GetProperty(a_property));
	}

	template <class T>
	inline void SetSetting(const RE::BSFixedString& a_property, T a_value)
	{
		RE::BSScript::PackValue(GetMCM()->GetProperty(a_property), a_value);
	}

	namespace Configuration
	{
		_NODISCARD bool IsValidPrerequisite();
		_NODISCARD bool IsValidTPLoc();
		_NODISCARD bool IsValidActor(RE::Actor* subject);
		_NODISCARD bool IsValidRace(RE::Actor* subject);

		_NODISCARD bool IsStripProtected(const RE::BGSKeywordForm* a_form);
		_NODISCARD bool IsDaedric(const RE::BGSKeywordForm* a_form);

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

	}  // namespace Configuration

}  // namespace Papyrus