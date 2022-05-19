#include "Kudasai/Defeat.h"

#include "Serialization/EventManager.h"

namespace Kudasai::Defeat
{
	void defeat(RE::Actor* subject, const bool skip_animation)
	{
		logger::info("Defeating Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
		Serialize::GetSingleton()->Defeated.emplace(subject->GetFormID());
		// ensure no1 attacc them
		pacify(subject);
		// render helpless
		if (subject->IsPlayerRef()) {
			// skipping lifestate for player, forces the bleedout camera which kinda meh
			using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
			auto cmap = RE::ControlMap::GetSingleton();
			cmap->ToggleControls(UEFlag::kActivate, false);
			cmap->ToggleControls(UEFlag::kJumping, false);
			cmap->ToggleControls(UEFlag::kMenu, false);
		} else {
			subject->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kBleedout;
			// apply npc package
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			auto args = RE::MakeFunctionArguments(std::move(subject));
			vm->DispatchStaticCall("KudasaiInternal", "FinalizeDefeat", args, callback);
		}
		// force bleedout
		subject->boolFlags.set(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
		if (subject->IsWeaponDrawn())
			SheatheWeapon(subject);
		if (!skip_animation) {
			SKSE::GetTaskInterface()->AddTask([subject]() {
				subject->NotifyAnimationGraph("BleedoutStart");
			});
		}
		// add keyword to identify in CK conditions
		const auto defeatkeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x7946FF, ESPNAME);
		AddKeyword(subject, defeatkeyword);

		Serialization::EventManager::GetSingleton()->_actordefeated.QueueEvent(subject);
	}

	void rescue(RE::Actor* subject, const bool undo_pacify, const bool skip_animation)
	{
		logger::info("Rescueing Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
		RescueImpl(subject);
		if (undo_pacify)
			UndoPacifyImpl(subject);

		subject->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kAlive;
		if (!skip_animation) {
			SKSE::GetTaskInterface()->AddTask([subject]() {
				// subject->NotifyAnimationGraph("bleedoutStop");
				subject->NotifyAnimationGraph("IdleForceDefaultState");
			});
		}

		Serialization::EventManager::GetSingleton()->_actorrescued.QueueEvent(subject);
	}


	void pacify(RE::Actor* subject)
	{
		logger::info("Pacyfying Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
		Serialize::GetSingleton()->Pacified.emplace(subject->GetFormID());
		// take out of combat
		SKSE::GetTaskInterface()->AddTask([subject]() {
			RE::ProcessLists::GetSingleton()->StopCombatAndAlarmOnActor(subject, false);
			subject->StopCombat();
		});
		// mark for CK Conditions
		const auto pacifykeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x7D1354, ESPNAME);
		AddKeyword(subject, pacifykeyword);
	}

	void undopacify(RE::Actor* subject)
	{
		logger::info("Undoing Pacify Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
		UndoPacifyImpl(subject);
	}

	bool isdefeated(RE::Actor* subject)
	{
		const auto srl = Serialize::GetSingleton();
		const auto key = subject->GetFormID();
		return srl->Defeated.contains(key) && srl->Pacified.contains(key);
	}

	bool ispacified(RE::Actor* subject)
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

	void RescueImpl(RE::Actor* subject)
	{
		const auto Srl = Serialize::GetSingleton();
		if (Srl->Defeated.erase(subject->GetFormID()) == 0)
			return;

		subject->boolFlags.reset(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
		if (subject->IsPlayerRef()) {
			using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
			auto cmap = RE::ControlMap::GetSingleton();
			cmap->ToggleControls(UEFlag::kActivate, true);
			cmap->ToggleControls(UEFlag::kJumping, true);
			cmap->ToggleControls(UEFlag::kMenu, true);
		} else {
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			auto args = RE::MakeFunctionArguments(std::move(subject));
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			vm->DispatchStaticCall("KudasaiInternal", "FinalizeRescue", args, callback);
		}
		// keyword for CK Conditions
		const auto defeatkeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x7946FF, ESPNAME);
		RemoveKeyword(subject, defeatkeyword);
	}

	void UndoPacifyImpl(RE::Actor* subject)
	{
		if (Serialize::GetSingleton()->Pacified.erase(subject->GetFormID()) == 0)
			return;
		// keyword for CK Conditions
		const auto pacifykeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x7D1354, ESPNAME);
		RemoveKeyword(subject, pacifykeyword);
	}

}  // namespace Defeat
