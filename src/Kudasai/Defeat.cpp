
#include "Kudasai/Defeat.h"

namespace Kudasai
{
	namespace Defeat
	{
		void defeatactor(RE::Actor* subject, const bool forcebleedout)
		{
			// TODO: !IMPORTANT w/e calls this needs to not call it if the attacker is in killmove
			logger::info("Defeating Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
			/* a defeated actor needs to..
        - have all their hits negated & ignored
        - ignore surrounding combat
        - be ignored by surrounding combat
      */
			Srl::GetSingleton()->defeats.emplace(subject->GetFormID());

			RE::ProcessLists::GetSingleton()->StopCombatAndAlarmOnActor(subject, false);
			subject->StopCombat();
			if (subject->IsPlayerRef()) {
				using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
				auto cmap = RE::ControlMap::GetSingleton();
				cmap->ToggleControls(UEFlag::kActivate, false);
				cmap->ToggleControls(UEFlag::kJumping, false);
				cmap->ToggleControls(UEFlag::kMenu, false);
			}
			if (forcebleedout) {
				// bleedout & do nothin package
				auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
				auto args = RE::MakeFunctionArguments(std::move(subject));
				RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
				vm->DispatchStaticCall("Kudasai", "ForceBleedout", args, callback);
			}

			logger::info("Defeated Actor, finalized");
		}

		void restoreactor(RE::Actor* subject, const bool rescue)
		{
			Srl::GetSingleton()->defeats.erase(subject->GetFormID());
			subject->boolFlags.reset(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
			if (subject->IsPlayerRef()) {
				using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
				auto cmap = RE::ControlMap::GetSingleton();
				cmap->ToggleControls(UEFlag::kActivate, true);
				cmap->ToggleControls(UEFlag::kJumping, true);
				cmap->ToggleControls(UEFlag::kMenu, true);
			}
			if (rescue) {
				auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
				auto args = RE::MakeFunctionArguments(std::move(subject));
				RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
				vm->DispatchStaticCall("Kudasai", "ClearBleedout", args, callback);
			}
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
