#include "Papyrus/Functions.h"

#include "Kudasai/Animation/Animation.h"
#include "Kudasai/Combat/Resolution.h"
#include "Kudasai/Defeat.h"
#include "Kudasai/Interface/QTE.h"
#include "Kudasai/Misc.h"
#include "Papyrus/Property.h"
#include "Papyrus/Settings.h"

namespace Papyrus
{
	void Defeat::DefeatActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, bool skip_animation)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot Defeat a none Actor", a_stackID);
			return;
		}
		SKSE::GetTaskInterface()->AddTask([=]() {
			Kudasai::Defeat::defeat(subject, skip_animation);
		});
	}

	void Defeat::RescueActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, bool undopacify, bool skip_animation)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot Rescue a none Actor", a_stackID);
			return;
		}
		SKSE::GetTaskInterface()->AddTask([=]() {
			Kudasai::Defeat::rescue(subject, undopacify, skip_animation);
		});
	}

	void Defeat::PacifyActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot Pacify a none Actor", a_stackID);
			return;
		}
		Kudasai::Defeat::pacify(subject);
	}

	void Defeat::UndoPacify(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot reset Pacification. Actor is none.", a_stackID);
			return;
		}
		Kudasai::Defeat::undopacify(subject);
	}

	bool Defeat::IsDefeated(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot check Defeat Status. Actor is none", a_stackID);
			return false;
		}
		return Kudasai::Defeat::isdefeated(subject);
	}

	bool Defeat::IsPacified(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot check Pacification. Actor is none", a_stackID);
			return false;
		}
		return Kudasai::Defeat::ispacified(subject);
	}

	std::vector<RE::Actor*> Defeat::GetDefeated(RE::StaticFunctionTag*)
	{
		const auto& d = Serialize::GetSingleton()->Defeated;
		std::vector<RE::Actor*> ret;
		ret.reserve(d.size());
		for (const auto& e : d) {
			const auto& form = RE::TESForm::LookupByID(e);
			if (form)
				ret.push_back(form->As<RE::Actor>());
		};
		return ret;
	}

	void ObjectRef::SetLinkedRef(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* object, RE::TESObjectREFR* target, RE::BGSKeyword* keyword)
	{
		if (!object) {
			a_vm->TraceStack("Cannot set Linked Ref. Source is none", a_stackID);
			return;
		}
		object->extraList.SetLinkedRef(target, keyword);
	}


	std::vector<RE::TESObjectARMO*> Actor::GetWornArmor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, uint32_t ignoredmasks)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot get worn armor from a none reference", a_stackID);
			return {};
		}
		return Kudasai::GetWornArmor(subject, ignoredmasks);
	}

	std::vector<RE::TESObjectARMO*> Actor::StripActor(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, uint32_t ignoredmasks)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot strip a none reference", a_stackID);
			return {};
		}
		const auto em = RE::ActorEquipManager::GetSingleton();
		const auto armors = Kudasai::GetWornArmor(subject, ignoredmasks);
		std::vector<RE::TESObjectARMO*> ret{};
		for (auto& armor : armors) {
			if (Configuration::IsStripProtected(armor)) {
				continue;
			}
			em->UnequipObject(subject, armor);
			ret.push_back(armor);
		}
		return ret;
	}

	void ObjectRef::RemoveAllItems(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* from, RE::TESObjectREFR* to, bool excludeworn)
	{
		if (!from) {
			a_vm->TraceStack("Cannot remove Items from a none reference", a_stackID);
			return;
		}

		auto reason = [&]() {
			using REASON = RE::ITEM_REMOVE_REASON;
			if (!to)
				return REASON::kRemove;
			else if (auto actor = to->As<RE::Actor>(); actor)
				return actor->IsPlayerTeammate() ? REASON::kStoreInTeammate : REASON::kSteal;
			else
				return REASON::kStoreInContainer;
		}();

		auto inventory = from->GetInventory();
		for (const auto& [form, data] : inventory) {
			if (!form->GetPlayable() || form->GetName()[0] == '\0')
				continue;
			else if (data.second->IsQuestObject())
				continue;
			else if (data.second->IsWorn() && excludeworn)
				if (auto kywdform = form->As<RE::BGSKeywordForm>(); kywdform && Configuration::IsStripProtected(kywdform))
					continue;

			from->RemoveItem(form, data.first, reason, nullptr, to, 0, 0);
		}
	}

	RE::AlchemyItem* Actor::GetMostEfficientPotion(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, RE::TESObjectREFR* container)
	{
		using Flag = RE::EffectSetting::EffectSettingData::Flag;
		if (!subject) {
			a_vm->TraceStack("Actor is none", a_stackID);
			return nullptr;
		} else if (!container) {
			a_vm->TraceStack("Container Reference is none", a_stackID);
			return nullptr;
		}
		const float tmphp = subject->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kHealth);
		const float maxhp = subject->GetPermanentActorValue(RE::ActorValue::kHealth) + tmphp;
		const float missinghp = maxhp - subject->GetActorValue(RE::ActorValue::kHealth);
		RE::AlchemyItem* ret = nullptr;
		float closest = FLT_MAX;
		const auto inventory = container->GetInventory();
		for (const auto& [form, data] : inventory) {
			if (data.first <= 0 || data.second->IsQuestObject() || !form->Is(RE::FormType::AlchemyItem))
				continue;
			const auto potion = form->As<RE::AlchemyItem>();
			if (potion->IsFood())
				continue;

			const float healing = [&potion]() {
				float ret = 0.0f;
				for (auto& e : potion->effects) {
					const auto base = e->baseEffect;
					if (!base)
						continue;

					const auto& effectdata = base->data;
					if (effectdata.flags.any(Flag::kDetrimental, Flag::kHostile)) {
						ret = 0.0f;
						break;
					} else if (effectdata.flags.none(Flag::kRecover)) {
						if (effectdata.primaryAV == RE::ActorValue::kHealth)
							ret += e->effectItem.magnitude;
						else if (effectdata.secondaryAV == RE::ActorValue::kHealth)
							ret += e->effectItem.magnitude * effectdata.secondAVWeight;
					}
				}
				return ret;
			}();
			if (healing > 0.0f) {
				const auto result = fabs(missinghp - healing);
				if (result < closest) {
					closest = result;
					ret = potion;
				}
			}
		}
		return ret;
	}

	bool Config::ValidRace(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot validate Race of a none Actor", a_stackID);
			return false;
		}
		return Configuration::IsValidRace(subject);
	}

	bool Config::IsInterested(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, RE::Actor* partner)
	{
		if (!subject) {
			a_vm->TraceStack("Cannot check interest. Subject is none", a_stackID);
			return false;
		} else if (!partner) {
			a_vm->TraceStack("Cannot check interest. Partner is none", a_stackID);
			return false;
		}
		return Configuration::IsInterested(subject, partner);
	}

	std::vector<RE::TESObjectARMO*> Utility::RemoveArmorByKeyword(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::TESObjectARMO*> array, RE::BGSKeyword* keyword)
	{
		if (!keyword) {
			a_vm->TraceStack("Cannot filter against a none Keyword", a_stackID);
			return {};
		}
		auto it = std::remove_if(array.begin(), array.end(), [&](RE::TESObjectARMO* armor) { return armor && armor->HasKeyword(keyword); });
		array.erase(it, array.end());
		return array;
	}

	void Config::UpdateSettings(RE::StaticFunctionTag*)
	{
		Papyrus::Settings::GetSingleton()->UpdateSettings();
	}

	std::string Actor::GetRaceType(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* akActor)
	{
		if (!akActor) {
			a_vm->TraceStack("Actor is none.", a_stackID);
			return ""s;
		}
		return Kudasai::Animation::GetRaceType(akActor);
	}

	void Utility::CreateFuture(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, float duration, RE::TESForm* callback, std::vector<RE::Actor*> argActor, int32_t argNum, RE::BSFixedString argStr)
	{
		if (!callback) {
			a_vm->TraceStack("Callback is none", a_stackID);
			return;
		}
		if (duration < 0.0f) {
			duration = 0.0f;
		}
		std::thread([=]() mutable {
			auto args = RE::MakeFunctionArguments(std::move(argActor), std::move(argNum), std::move(argStr));
			auto handle = a_vm->GetObjectHandlePolicy()->GetHandleForObject(callback->GetFormType(), callback);
			std::this_thread::sleep_for(std::chrono::milliseconds(lroundf(duration * 1000)));
			a_vm->SendEvent(handle, RE::BSFixedString{ "OnFuture_c" }, args);
		}).detach();
	}

	std::vector<std::string> Struggle::LookupStruggleAnimations(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions)
	{
		if (positions.empty()) {
			a_vm->TraceStack("Array is empty", a_stackID);
			return {};
		}
		try {
			return Kudasai::Animation::LookupStruggleAnimations(positions);
		} catch (const std::exception& e) {
			Kudasai::ConsolePrint("[Kudasai] Failed to retrieve Animations for Actors.");
			logger::error(e.what());
		}
		return {};
	}

	std::vector<std::string> Struggle::LookupBreakfreeAnimations(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions)
	{
		if (positions.empty()) {
			a_vm->TraceStack("Array is empty", a_stackID);
			return {};
		}
		return Kudasai::Animation::LookupBreakfreeAnimations(positions);
	}

	std::vector<std::string> Struggle::LookupKnockoutAnimations(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions)
	{
		if (positions.empty()) {
			a_vm->TraceStack("Array is empty", a_stackID);
			return {};
		}
		return Kudasai::Animation::LookupKnockoutAnimations(positions);
	}

	void Struggle::SetPositions(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions)
	{
		if (positions.empty()) {
			a_vm->TraceStack("Array is empty", a_stackID);
			return;
		}
		return Kudasai::Animation::SetPositions(positions);
	}

	void Struggle::ClearPositions(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions)
	{
		if (positions.empty()) {
			a_vm->TraceStack("Array is empty", a_stackID);
			return;
		}
		return Kudasai::Animation::ClearPositions(positions);
	}

	bool Interface::OpenQTEMenu(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, int32_t difficulty, RE::TESForm* callback)
	{
		if (!callback) {
			a_vm->TraceStack("Callback is none", a_stackID);
			return false;
		}

		const auto callbackfunc = [a_vm, callback](bool victory) {
			auto args = RE::MakeFunctionArguments(std::move(victory));
			auto handle = a_vm->GetObjectHandlePolicy()->GetHandleForObject(callback->GetFormType(), callback);
			a_vm->SendEvent(handle, RE::BSFixedString{ "OnQTEEnd_c" }, args);
		};
		return Kudasai::Interface::QTE::OpenMenu(difficulty, callbackfunc);
	}

	void Interface::CloseQTEMenu(RE::StaticFunctionTag*)
	{
		return Kudasai::Interface::QTE::CloseMenu(true);
	}

	bool Config::IsAlternateVersion(RE::StaticFunctionTag*)
	{
		return !Kudasai::IsLight();
	}

	std::vector<RE::Actor*> Actor::GetFollowers(RE::StaticFunctionTag*)
	{
		return Kudasai::GetFollowers();
	}

	void Utility::DisableProcessing(RE::StaticFunctionTag*, bool disable)
	{
		Papyrus::AllowProcessing = !disable;
	}

	void Utility::DisableConsequence(RE::StaticFunctionTag*, bool disable)
	{
		Papyrus::AllowConsequence = !disable;
	}

	bool Utility::IsProcessingDisabled(RE::StaticFunctionTag*)
	{
		return !Papyrus::AllowProcessing;
	}

	bool Utility::IsConsequenceDisabled(RE::StaticFunctionTag*)
	{
		return !Papyrus::AllowConsequence;
	}

	void Utility::SortByDistance(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> array, RE::TESObjectREFR* center)
	{
		if (!center) {
			a_vm->TraceStack("Center is none", a_stackID);
			return;
		} else if (array.empty()) {
			a_vm->TraceStack("Cannot sort an Empty Array", a_stackID);
			return;
		}
		Kudasai::SortByDistance(array, center);
	}

	void Utility::SortByDistanceRef(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> array, RE::TESObjectREFR* center)
	{
		if (!center) {
			a_vm->TraceStack("Center is none", a_stackID);
			return;
		} else if (array.empty()) {
			a_vm->TraceStack("Cannot sort an Empty Array", a_stackID);
			return;
		}
		Kudasai::SortByDistance(array, center);
	}


}  // namespace Papyrus
