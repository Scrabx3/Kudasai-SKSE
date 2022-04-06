#include "Papyrus/Functions.h"

#include "Kudasai/Defeat.h"
#include "Papyrus/Settings.h"
namespace Papyrus
{
	// Kudasai::Defeat

	void DefeatActor(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		Kudasai::Defeat::defeat(subject);
	}

	void RescueActor(RE::StaticFunctionTag*, RE::Actor* subject, bool undopacify)
	{
		Kudasai::Defeat::rescue(subject, undopacify);
	}

	void PacifyActor(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		Kudasai::Defeat::pacify(subject);
	}

	void UndoPacify(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		Kudasai::Defeat::undopacify(subject);
	}

	bool IsDefeated(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		return Kudasai::Defeat::isdefeated(subject);
	}

	bool IsPacified(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		return Kudasai::Defeat::ispacified(subject);
	}

	// Actor

	void SetLinkedRef(RE::StaticFunctionTag*, RE::TESObjectREFR* object, RE::TESObjectREFR* target, RE::BGSKeyword* keyword)
	{
		object->extraList.SetLinkedRef(target, keyword);
	}


	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::StaticFunctionTag*, RE::Actor* subject, bool ignore_config)
	{
		return Kudasai::GetWornArmor(subject, ignore_config);
	}

	void RemoveAllItems(RE::StaticFunctionTag*, RE::TESObjectREFR* from, RE::TESObjectREFR* to, bool excludeworn, int minvalue)
	{
		auto reason = [&]() {
			using REASON = RE::ITEM_REMOVE_REASON;
			if (!to)
				return REASON::kRemove;
			else if (to->Is(RE::FormType::ActorCharacter)) {
				auto actor = static_cast<RE::Actor*>(to);
				if (actor->IsPlayerTeammate())
					return REASON::kStoreInTeammate;
				else
					return REASON::kSteal;
			}
			return REASON::kStoreInContainer;
		}();

		auto inventory = from->GetInventory();
		for (const auto& [form, data] : inventory) {
			if (data.second->IsQuestObject())
				continue;
			else if (data.second->IsWorn() && excludeworn)
				continue;
			else if (data.second->GetValue() < minvalue)
				continue;

			from->RemoveItem(form, data.first, reason, nullptr, to, 0, 0);
		}
	}

	// Config

	bool ValidRace(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		return Configuration::isvalidrace(subject);
	}

	bool IsInterrested(RE::StaticFunctionTag*, RE::Actor* primus, std::vector<RE::Actor*> secundi)
	{
		return Configuration::isinterested(primus, secundi);
	}


	// Internal

	void SetDamageImmune(RE::StaticFunctionTag*, RE::Actor* subject, bool immune)
	{
		Kudasai::Defeat::setdamageimmune(subject, immune);
	}

}  // namespace Papyrus
