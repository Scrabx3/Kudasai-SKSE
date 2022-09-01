#pragma once

namespace Kudasai
{
	// Debug
	void ConsolePrint(std::string a_msg);

	void WinMsgFATAL(const char* a_msg, const char* a_cpt);

	bool IsLight();

	// Actor
	float getavpercent(RE::Actor* a_actor, RE::ActorValue a_val);

	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::Actor* a_actor, bool ignore_config);

	std::vector<RE::Actor*> GetFollowers();

	void SetVehicle(RE::Actor* actor, RE::TESObjectREFR* vehicle);

	void ResetVehicle(RE::Actor* subject);

	void SetPlayerAIDriven(bool aidriven = true);

	void SetRestrained(RE::Actor* subject, bool restrained);

	void StopTranslating(RE::Actor* subject);

	void AddKeyword(RE::Actor* subject, RE::BGSKeyword* keyword, bool add = true);

	void RemoveKeyword(RE::Actor* subject, RE::BGSKeyword* keyword);

	void SheatheWeapon(RE::Actor* subject);

	void RemoveFromFaction(RE::Actor* subject, RE::TESFaction* faction);

	// ObjectReference
	RE::TESObjectREFR* PlaceAtMe(RE::TESObjectREFR* where, RE::TESForm* what, std::uint32_t count = 1, bool forcePersist = false, bool initiallyDisabled = false);

	template <class T>
	void SortByDistance(std::vector<T> array, RE::TESObjectREFR* center)
	{
		const auto p = center->GetPosition();
		std::sort(array.begin(), array.end(), [&](T& first, T& second) {
			return first->GetPosition().GetDistance(p) < second->GetPosition().GetDistance(p);
		});
	}

}  // namespace Kudasai
