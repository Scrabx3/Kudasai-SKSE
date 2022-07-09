#pragma once

#include "Papyrus/Property.h"

namespace Papyrus
{
	// namespace Settings
	// {
	// 	void UpdateSettings();

	// 	inline static bool bEnabled = false;
	// 	inline static bool bCreatureDefeat;

	// 	inline static bool bLethalEssential;
	// 	inline static float fLethalPlayer;
	// 	inline static float fLethalNPC;

	// 	inline static int iStripReq;
	// 	inline static float fStripReqChance;
	// 	inline static float fStripChance;
	// 	inline static float fStripDestroy;
	// 	inline static bool bStripDrop;

	// 	inline static int iStrips;
	// 	inline static bool bNotifyDefeat;
	// 	inline static bool bNotifyDestroy;
	// 	inline static bool bNotifyColored;
	// };

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
			std::vector<RE::FormID> prLCTN;	 // prequisite forms
			std::vector<RE::FormID> tpLCTN;	 // no teleport forms
			std::vector<RE::FormID> exNPC_;	 // excluded forms

			std::vector<RE::FormID> armKYWD;  // strip destruction procecc
		};
	};	// class Configuration

}  // namespace Papyrus