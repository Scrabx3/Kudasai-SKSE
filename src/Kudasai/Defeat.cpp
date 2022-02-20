
#include "Kudasai/Defeat.h"

namespace Kudasai
{
	namespace Defeat
	{
		void defeatactor(RE::Actor* subject)
		{
			logger::info("Defeating Actor: {}", subject->GetFormID());
			/* a defeated actor needs to..
        - have all their hits negated & ignored
        - ignore surrounding combat
        - be ignored by surrounding combat
      */
			auto pl = RE::ProcessLists::GetSingleton();
			if (pl) {
				logger::info("Stop combat + alarm on actor >> {}", subject->GetFormID());
				pl->StopCombatAndAlarmOnActor(subject, false);
			}
			Srl::GetSingleton()->defeats.emplace(subject->GetFormID());

			// if (subject->IsBleedingOut())
			// 	return;
			// logger::info("Forcing Bleedout");
			// subject->NotifyAnimationGraph("bleedoutStart");
			// subject->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kBleedout;
		}

		void restoreactor(RE::Actor* subject)
		{
			auto process = subject->currentProcess;
			if (process)
				process->ignoringCombat = false;
		}

		bool isdefeated(RE::Actor* subject)
		{
			return Srl::GetSingleton()->defeats.contains(subject->GetFormID());
		}
	}  // namespace Defeat

}  // namespace Kudasai
