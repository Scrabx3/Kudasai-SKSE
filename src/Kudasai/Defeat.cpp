
#include "Kudasai/Defeat.h"

namespace Kudasai
{
	namespace Defeat
	{
		void defeatactor(RE::Actor* subject, const bool forcebleedout)
		{
			logger::info("Defeating Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
			/* a defeated actor needs to..
        - have all their hits negated & ignored
        - ignore surrounding combat
        - be ignored by surrounding combat
      */
			Srl::GetSingleton()->defeats.emplace(subject->GetFormID());
			RE::ProcessLists::GetSingleton()->StopCombatAndAlarmOnActor(subject, false);
			subject->StopCombat();

			if (forcebleedout)
				enforcebleedout(subject);

			logger::info("Defeated Actor, finalized");
		}

		void enforcebleedout(RE::Actor* subject)
		{
			// bleedout & do nothin package
			if (subject->IsPlayerRef()) {
				using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
				auto cmap = RE::ControlMap::GetSingleton();
				cmap->ToggleControls(UEFlag::kActivate, false);
				cmap->ToggleControls(UEFlag::kJumping, false);
				cmap->ToggleControls(UEFlag::kMenu, false);
			}
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			subject->boolFlags.set(RE::Actor::BOOL_FLAGS::kInBleedoutAnimation);
			bool playanim = (subject->IsPlayerRef() || subject->boolFlags.none(RE::Actor::BOOL_FLAGS::kInBleedoutAnimation)) && !subject->IsBleedingOut();
			auto args = RE::MakeFunctionArguments(std::move(subject), std::move(playanim));
			vm->DispatchStaticCall("KudasaiInternal", "ForceBleedout", args, callback);
		}

		void restoreactor(RE::Actor* subject, const bool rescue)
		{
			Srl::GetSingleton()->defeats.erase(subject->GetFormID());
			subject->boolFlags.reset(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
			if (rescue)
				clearbleedout(RE::Actor* subject);
		}

		void clearbleedout(RE::Actor* subject)
		{
			if (subject->IsPlayerRef()) {
				using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
				auto cmap = RE::ControlMap::GetSingleton();
				cmap->ToggleControls(UEFlag::kActivate, true);
				cmap->ToggleControls(UEFlag::kJumping, true);
				cmap->ToggleControls(UEFlag::kMenu, true);
			}
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			auto args = RE::MakeFunctionArguments(std::move(subject));
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			vm->DispatchStaticCall("KudasaiInternal", "ClearBleedout", args, callback);
		}

		bool isdefeated(RE::Actor* subject)
		{
			auto srl = Srl::GetSingleton()->defeats;
			if (srl.empty())
				return false;
			return Srl::GetSingleton()->defeats.contains(subject->GetFormID());
		}
	}  // namespace Defeat

}  // namespace Kudasai
