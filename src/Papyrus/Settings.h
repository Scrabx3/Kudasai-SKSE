#pragma once

#include "Papyrus/Property.h"

namespace Papyrus
{
	template <class T>
	inline T GetSetting(RE::BSFixedString property)
	{
		static const auto mcm = []() {
			const auto form = RE::TESDataHandler::GetSingleton()->LookupForm(0x7853F1, ESPNAME);
			return CreateObjectPtr(form, "KudasaiMCM");
		}();
		return RE::BSScript::UnpackValue<T>(mcm->GetProperty(property));
	}

	template <class T>
	inline void SetSetting(RE::BSFixedString property, T val)
	{
		const auto form = RE::TESDataHandler::GetSingleton()->LookupForm(0x7853F1, ESPNAME);
		auto var = CreateObjectPtr(form, "KudasaiMCM")->GetProperty(property);
		RE::BSScript::PackValue(var, val);
	}

	namespace Configuration
	{
		_NODISCARD const bool IsValidActor(RE::Actor* subject);	 // check if Actor is valid, as in not excluded
		_NODISCARD const bool IsValidRace(RE::Actor* subject);	 // validate racekey

		_NODISCARD const bool IsInterested(RE::Actor* subject, RE::Actor* partner);					 // validate partners sexuality
		_NODISCARD const bool IsGroupAllowed(RE::Actor* subject, std::vector<RE::Actor*> partners);	 // check for valid grouping

		_NODISCARD const bool HasSchlong(RE::Actor* subject);
		_NODISCARD const bool IsNPC(RE::Actor* subject);

		struct Data :
			public Singleton<Data>
		{
			void LoadData();

			std::set<RE::FormID> excluded;		 // Excluded Actors (Base Actor Forms only)
		};
	};	// class Configuration

}  // namespace Papyrus