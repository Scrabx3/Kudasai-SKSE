#pragma once

namespace Serialize
{

	inline void ApplyKeywordSet(std::set<RE::FormID> list, RE::BGSKeyword* keyword)
	{
		for (auto& id : list) {
			auto actor = RE::TESForm::LookupByID<RE::Actor>(id);
			if (actor)
				Kudasai::AddKeyword(actor, keyword);
		}
	}

	inline void RemoveKeywordSet(std::set<RE::FormID> list, RE::BGSKeyword* keyword)
	{
		for (auto& id : list) {
			auto actor = RE::TESForm::LookupByID<RE::Actor>(id);
			if (actor)
				Kudasai::RemoveKeyword(actor, keyword);
		}
	}
}  // namespace Serialize
