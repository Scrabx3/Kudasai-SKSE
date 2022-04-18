#include "Kudasai/Defeat.h"

#include "Serialization/EventManager.h"

namespace Kudasai::Defeat
{
	void defeat(RE::Actor* subject)
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
		auto task = SKSE::GetTaskInterface();
		task->AddTask([subject]() {
			subject->boolFlags.set(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
			if (subject->IsWeaponDrawn())
				SheatheWeapon(subject);
			subject->NotifyAnimationGraph("BleedoutStart");
		});
		// add keyword to identify in CK conditions
		const auto defeatkeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x7946FF, ESPNAME);
		AddKeyword(subject, defeatkeyword);

		Serialization::EventManager::GetSingleton()->_actordefeated.QueueEvent(subject);
	}

	void rescue(RE::Actor* subject, const bool undo_pacify)
	{
		logger::info("Rescueing Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
		if (Serialize::GetSingleton()->Defeated.erase(subject->GetFormID()) == 0)
			return;
		// let them stand up
		subject->boolFlags.reset(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
		auto task = SKSE::GetTaskInterface();
		task->AddTask([subject]() {
			subject->NotifyAnimationGraph("BleedoutStop");
		});
		// remove restrictions
		subject->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kAlive;
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
		// clear pacification
		if (undo_pacify)
			undopacify(subject);
		// remove keyword for CK conditions
		const auto defeatkeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x7946FF, ESPNAME);
		RemoveKeyword(subject, defeatkeyword);

		Serialization::EventManager::GetSingleton()->_actorrescued.QueueEvent(subject);
	}


	void pacify(RE::Actor* subject)
	{
		logger::info("Pacyfying Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
		Serialize::GetSingleton()->Pacified.emplace(subject->GetFormID());
		// take out of combat
		auto task = SKSE::GetTaskInterface();
		task->AddTask([subject]() {
			RE::ProcessLists::GetSingleton()->StopCombatAndAlarmOnActor(subject, false);
			subject->StopCombat();
		});
		// mark for CK Conditions
		auto handler = RE::TESDataHandler::GetSingleton();
		const auto pacifykeyword = handler->LookupForm<RE::BGSKeyword>(0x7D1354, ESPNAME);
		AddKeyword(subject, pacifykeyword);
	}

	void undopacify(RE::Actor* subject)
	{
		logger::info("Undoing Pacify Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
		if (Serialize::GetSingleton()->Pacified.erase(subject->GetFormID()) == 0)
			return;
		// remove keyword for CK Conditions
		auto handler = RE::TESDataHandler::GetSingleton();
		const auto pacifykeyword = handler->LookupForm<RE::BGSKeyword>(0x7D1354, ESPNAME);
		RemoveKeyword(subject, pacifykeyword);
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

}  // namespace Defeat
