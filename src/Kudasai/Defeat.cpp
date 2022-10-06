#include "Kudasai/Defeat.h"

#include "Serialization/EventManager.h"
#include "Kudasai/EventSink.h"

namespace Kudasai::Defeat
{
	void defeat(RE::Actor* subject, const bool skip_animation)
	{
		if (subject->IsDead()) {
			return;
		}
		logger::info("Defeating Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
		Serialize::GetSingleton()->Defeated.emplace(subject->GetFormID());

		const auto setteammtes = [](const std::vector<RE::Actor*>& followers, bool toggle) {
			for (auto& e : followers) {
				toggle ? e->boolBits.set(RE::Actor::BOOL_BITS::kPlayerTeammate) : e->boolBits.reset(RE::Actor::BOOL_BITS::kPlayerTeammate);
			}
		};
		pacify(subject);
		// render helpless & pacify
		if (subject->IsPlayerRef()) {
			const auto fols = GetFollowers();
			// Remove Follower Flag to avoid aggression resets upon defeat
			setteammtes(fols, false);
			using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
			auto cmap = RE::ControlMap::GetSingleton();
			cmap->ToggleControls(UEFlag::kActivate, false);
			cmap->ToggleControls(UEFlag::kJumping, false);
			cmap->ToggleControls(UEFlag::kMainFour, false);
			EventHandler::RegisterAnimSink(subject, true);
			// Only do this for the Player, NPC AI may glitch out otherwise
			if (subject->IsWeaponDrawn())
				SheatheWeapon(subject);
			setteammtes(fols, true);
		} else {
			if (subject->IsPlayerTeammate()) {
				subject->SetActorValue(RE::ActorValue::kWaitingForPlayer, 1);
			}
			// apply npc package
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			auto args = RE::MakeFunctionArguments(std::move(subject));
			vm->DispatchStaticCall("KudasaiInternal", "FinalizeDefeat", args, callback);
		}
		// force bleedout
		subject->boolFlags.set(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
		if (!skip_animation)
			subject->NotifyAnimationGraph("BleedoutStart");
		// add keyword to identify in CK conditions
		const auto defeatkeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(KeywordDefeat, ESPNAME);
		AddKeyword(subject, defeatkeyword);
		Serialization::EventManager::GetSingleton()->_actordefeated.QueueEvent(subject);
	}

	void rescue(RE::Actor* subject, const bool undo_pacify, const bool skip_animation)
	{
		logger::info("Rescuing Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
		const auto Srl = Serialize::GetSingleton();
		if (Srl->Defeated.erase(subject->GetFormID()) == 0)
			return;

		if (subject->IsPlayerRef()) {
			using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
			auto cmap = RE::ControlMap::GetSingleton();
			cmap->ToggleControls(UEFlag::kActivate, true);
			cmap->ToggleControls(UEFlag::kJumping, true);
			cmap->ToggleControls(UEFlag::kMainFour, true);
			EventHandler::RegisterAnimSink(subject, false);
		} else {
			subject->SetLifeState(RE::ACTOR_LIFE_STATE::kAlive);
			if (subject->IsPlayerTeammate()) {
				subject->SetActorValue(RE::ActorValue::kWaitingForPlayer, 0);
			}
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			auto args = RE::MakeFunctionArguments(std::move(subject));
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			vm->DispatchStaticCall("KudasaiInternal", "FinalizeRescue", args, callback);
		}
		if (undo_pacify)
			UndoPacifyImpl(subject);
		if (!subject->IsDead()) {
			subject->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kAlive;
			if (!skip_animation) {
				subject->NotifyAnimationGraph("BleedoutStop");
			}
		}
		// keyword for CK Conditions
		const auto defeatkeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(KeywordDefeat, ESPNAME);
		RemoveKeyword(subject, defeatkeyword);

		Serialization::EventManager::GetSingleton()->_actorrescued.QueueEvent(subject);
	}


	void pacify(RE::Actor* subject)
	{
		logger::info("Pacyfying Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
		Serialize::GetSingleton()->Pacified.emplace(subject->GetFormID());
		const auto process = RE::ProcessLists::GetSingleton();
		process->runDetection = false;
		process->ClearCachedFactionFightReactions();
		process->StopCombatAndAlarmOnActor(subject, false);
		subject->StopCombat();
		process->runDetection = true;
		// mark for CK Conditions
		const auto pacifykeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(KeywordPacify, ESPNAME);
		AddKeyword(subject, pacifykeyword);
	}

	void undopacify(RE::Actor* subject)
	{
		logger::info("Undoing Pacify Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
		UndoPacifyImpl(subject);
	}

	bool isdefeated(const RE::Actor* subject)
	{
		const auto srl = Serialize::GetSingleton();
		const auto key = subject->GetFormID();
		return srl->Defeated.contains(key) && srl->Pacified.contains(key);
	}

	bool ispacified(const RE::Actor* subject)
	{
		const auto srl = Serialize::GetSingleton();
		return srl->Pacified.contains(subject->GetFormID());
	}

	void SetDamageImmune(RE::Actor* subject)
	{
		const auto srl = Serialize::GetSingleton();
		srl->Defeated.emplace(subject->GetFormID());
	}

	bool IsDamageImmune(RE::Actor* subject)
	{
		const auto srl = Serialize::GetSingleton();
		return srl->Defeated.contains(subject->GetFormID());
	}

	void UndoPacifyImpl(RE::Actor* subject)
	{
		if (Serialize::GetSingleton()->Pacified.erase(subject->GetFormID()) == 0)
			return;
		// keyword for CK Conditions
		const auto pacifykeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(KeywordPacify, ESPNAME);
		RemoveKeyword(subject, pacifykeyword);
	}

}  // namespace Defeat
