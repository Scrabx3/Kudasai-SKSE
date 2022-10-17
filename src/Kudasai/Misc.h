#pragma once

namespace Kudasai
{
	// Debug
	void ConsolePrint(std::string a_msg);

	void WinMsgFATAL(const char* a_msg, const char* a_cpt);

	bool IsLight();

	// Form
	inline RE::FormID IDFromString(const std::string& formstring)
	{
		const auto split = formstring.find("|");
		const auto esp = formstring.substr(split + 1);
		const auto formid = std::stoi(formstring.substr(0, split), nullptr, 16);
		return RE::TESDataHandler::GetSingleton()->LookupFormID(formid, esp);
	}

	template <class T>
	T* FormFromString(const std::string& formstring)
	{
		const auto split = formstring.find("|");
		const auto esp = formstring.substr(split + 1);
		const auto formid = std::stoi(formstring.substr(0, split), nullptr, 16);
		return RE::TESDataHandler::GetSingleton()->LookupForm<T>(formid, esp);
	}

	// Actor
	float getavpercent(RE::Actor* a_actor, RE::ActorValue a_val);

	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::Actor* a_actor, uint32_t a_ignoredmasks = 0);

	std::vector<RE::Actor*> GetFollowers();

	void SetVehicle(RE::Actor* actor, RE::TESObjectREFR* vehicle);

	void ResetVehicle(RE::Actor* subject);

	void SetRestrained(RE::Actor* subject, bool restrained);

	void StopTranslating(RE::Actor* subject);

	void AddKeyword(RE::Actor* subject, RE::BGSKeyword* keyword, bool add = true);

	void RemoveKeyword(RE::Actor* subject, RE::BGSKeyword* keyword);

	void SheatheWeapon(RE::Actor* subject);

	void RemoveFromFaction(RE::Actor* subject, RE::TESFaction* faction);

	bool IsHunter(RE::Actor* a_actor);

	RE::TESActorBase* GetLeveledActorBase(RE::Actor* a_actor);

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

	// YAML
	template <class T>
	std::vector<T*> ReadFormsFromYaml(const YAML::Node& node)
	{
		if (!node.IsSequence())
			return {};
		auto list = node.as<std::vector<std::string>>();
		std::vector<T*> ret;
		ret.reserve(list.size());
		for (auto& str : list)
			if (auto itm = FormFromString<T>(str); itm)
				ret.push_back(itm);
		return ret;
	}

	template <class T>
	std::vector<T*> ReadFormsFromYaml(const YAML::Node& node, const char* attribute)
	{
		return node[attribute].IsDefined() ? ReadFormsFromYaml<T>(node[attribute]) : std::vector<T*>{};
	}

	inline std::vector<RE::FormID> ReadFormIDsFromYaml(const YAML::Node& node)
	{
		if (!node.IsSequence()) return {};
		auto list = node.as<std::vector<std::string>>();
		std::vector<RE::FormID> ret;
		ret.reserve(list.size());
		for (auto& str : list)
			if (auto itm = IDFromString(str); itm)
				ret.push_back(itm);
		return ret;
	}

	inline std::vector<RE::FormID> ReadFormIDsFromYaml(const YAML::Node& node, const char* attribute)
	{
		return node[attribute].IsDefined() ? ReadFormIDsFromYaml(node[attribute]) : std::vector<RE::FormID>{};
	}

	// String
	template <class T>
	constexpr void ToLower(T& str)
	{
		std::transform(str.cbegin(), str.cend(), str.begin(), [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
	}

	constexpr void ChipString(std::string& str)
	{
		str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char c) { return std::isspace(c); }), str.end());
	}

}  // namespace Kudasai
