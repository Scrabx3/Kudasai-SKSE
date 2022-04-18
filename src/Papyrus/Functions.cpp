#include "Papyrus/Functions.h"

#include "Kudasai/Combat/Resolution.h"
#include "Kudasai/Defeat.h"
#include "Kudasai/Misc.h"
#include "Kudasai/Struggle/Struggly.h"
#include "Papyrus/Property.h"
#include "Papyrus/Settings.h"
namespace Papyrus
{
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

	void SetLinkedRef(RE::StaticFunctionTag*, RE::TESObjectREFR* object, RE::TESObjectREFR* target, RE::BGSKeyword* keyword)
	{
		object->extraList.SetLinkedRef(target, keyword);
	}


	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::StaticFunctionTag*, RE::Actor* subject, bool ignore_config)
	{
		return Kudasai::GetWornArmor(subject, ignore_config);
	}

	void RemoveAllItems(RE::StaticFunctionTag*, RE::TESObjectREFR* from, RE::TESObjectREFR* to, bool excludeworn)
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

			from->RemoveItem(form, data.first, reason, nullptr, to, 0, 0);
		}
	}

	bool CreateStruggle(VM* vm, RE::VMStackID, RE::StaticFunctionTag*, RE::Actor* victim, RE::Actor* aggressor, float difficulty, RE::BSFixedString callback)
	{
		using ST = Kudasai::Struggle::StruggleType;

		const auto callbackfunc = [vm, callback](bool victimvictory, Kudasai::Struggle* struggle) {
			auto arg0 = struggle->actors;
			auto args = RE::MakeFunctionArguments(std::move(arg0), std::move(victimvictory));
			vm->SendEventAll(RE::BSFixedString{"KudasaiStruggle_"s + callback.c_str()}, args);
			std::thread(&Kudasai::Struggle::DeleteStruggle, struggle).detach();
		};
		try {
			ST type = victim->IsPlayerRef() || aggressor->IsPlayerRef() ? ST::QTE : ST::None;
			Kudasai::Struggle::CreateStruggle(callbackfunc, std::vector{ victim, aggressor }, difficulty, type);
			return true;
		} catch (const std::exception& e) {
			logger::warn("Failed to create Struggle Animation -> Error = {}", e.what());
			Kudasai::ConsolePrint("Failed to create Struggle Animation");
			return false;
		}
	}

	void PlayBreakfree(RE::StaticFunctionTag*, std::vector<RE::Actor*> positions)
	{
		Kudasai::Struggle::PlayBreakfree(positions);
	}

	void PlayBreakfreeCustom(RE::StaticFunctionTag*, std::vector<RE::Actor*> positions, std::vector<std::string> animations)
	{
		Kudasai::Struggle::PlayBreakfree(positions, animations);
	}

	bool IsStruggling(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		return Kudasai::Struggle::FindPair(subject) != nullptr;
	}

	bool StopStruggle(RE::StaticFunctionTag*, RE::Actor* victoire)
	{
		if (auto struggle = Kudasai::Struggle::FindPair(victoire); struggle) {
			struggle->StopStruggle(struggle->actors[0] == victoire ? struggle->actors[1] : victoire);
			return true;
		} else {
			return false;
		}
	}

	bool StopStruggleReverse(RE::StaticFunctionTag*, RE::Actor* defeated)
	{
		if (auto struggle = Kudasai::Struggle::FindPair(defeated); struggle) {
			struggle->StopStruggle(defeated);
			return true;
		} else {
			return false;
		}
	}


	bool ValidRace(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		return Configuration::IsValidRace(subject);
	}

	bool IsInterested(RE::StaticFunctionTag*, RE::Actor* subject, RE::Actor* partner)
	{
		return Configuration::IsInterested(subject, partner);
	}

	bool IsGroupAllowed(RE::StaticFunctionTag*, RE::Actor* subject, std::vector<RE::Actor*> partners)
	{
		return Configuration::IsGroupAllowed(subject, partners);
	}

	// Internal
	void UpdateWeights(RE::StaticFunctionTag*)
	{
		Kudasai::Resolution::UpdateWeights();
	}

}  // namespace Papyrus
