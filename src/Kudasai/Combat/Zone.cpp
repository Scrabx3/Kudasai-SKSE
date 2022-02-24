#include "Kudasai/Combat/Zone.h"
#include "Kudasai/Defeat.h"

namespace Kudasai
{
	bool Zone::registerdefeat(RE::Actor* victim, RE::Actor* aggressor)
	{
		logger::info("<registerdefeat> Victim = {} <<->> Aggressor = {}", victim->GetFormID(), aggressor->GetFormID());
		auto agrzone = aggressor->GetCombatGroup();
		// auto viczone = victim->GetCombatGroup();
		if (!agrzone) {	 // || !viczone) {
			logger::warn("Failed to register defeat, aggressors Combat Group is missing");
			return false;
		}
		// victim cant be commanded, hit evaluation already checks that
		if (aggressor->IsCommandedActor()) {
			auto tmp = aggressor->GetCommandingActor().get();
			if (tmp) {
				logger::info("<registerdefeat> Aggressor is summon, using commander as aggressor = {}", tmp->GetFormID());
				aggressor = tmp.get();
			} else {
				logger::warn("<registerdefeat> Aggressor is commanded but no commander found? Abandon");
				return false;
			}
		}
		auto zone = GetSingleton();
		auto dtype = zone->getdefeattype(agrzone);
		if (dtype == DefeatResult::Cancel)
			return false;
		std::thread(&Zone::defeat, zone, victim, aggressor, dtype).detach();
		return true;
	}

	Zone::DefeatResult Zone::getdefeattype(RE::CombatGroup* agrzone)
	{
		using Res = Zone::DefeatResult;
		int agrnum = countvalid(agrzone->members);
		int tarnum = countvalid(agrzone->targets);
		if (agrnum == 0 || tarnum == 0) {
			logger::warn("Invalid validation count? agrnum = {} <<>> tarnum = {}", agrnum, tarnum);
			return Res::Cancel;
		}
		logger::info("agrnum = {} <<>> tarnum = {}", agrnum, tarnum);
		if (tarnum == 1) {
			logger::info("Target is final victim; returning type Resolution");
			// return Res::Resolution;
			return Res::Defeat;
		} else if (agrnum * 2 <= tarnum) {
			logger::info("Aggressor is outmatched; returning type Defeat");
			return Res::Defeat;
		}
		logger::info("Returning type Assault");
		// return Res::Assault;
		return Res::Defeat;
	}
	int Zone::countvalid(RE::BSTArray<RE::CombatGroup::TargetData> list)
	{
		int ret = 0;
		for (auto& target : list) {
			auto that = target.targetHandle.get();
			if (that && !that->IsCommandedActor() && !Defeat::isdefeated(that.get()))
				ret++;
		}
		return ret;
	}
	int Zone::countvalid(RE::BSTArray<RE::CombatGroup::MemberData> list)
	{
		int ret = 0;
		for (auto& target : list) {
			auto that = target.handle.get();
			if (that && !that->IsCommandedActor() && !Defeat::isdefeated(that.get()))
				ret++;
		}
		return ret;
	}

	void Zone::defeat(RE::Actor* victim, RE::Actor* aggressor, DefeatResult result)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(450));
		std::scoped_lock();
		if (victim->IsDead() || Defeat::isdefeated(victim) || victim->IsInKillMove()) {
			logger::info("Victim {} dead, defeated or in killmove", victim->GetFormID());
			return;
		}
		const auto process = victim->currentProcess;
		const auto middlehigh = process ? process->middleHigh : nullptr;
		if (middlehigh) {
			for (auto& data : middlehigh->commandedActors) {
				const auto eff = data.activeEffect;
				if (eff)
					victim->InvalidateCommandedActorEffect(eff);
			}
		}
		switch (result) {
		case DefeatResult::Resolution:
			// TODO: implement
			logger::warn("Invalid call to resolution");
			break;
		case DefeatResult::Assault:
			{
				logger::warn("Invalid call to assault");
				auto config = Papyrus::Configuration::GetSingleton();
				if (aggressor->IsDead() || Defeat::isdefeated(aggressor) || !config->isactorinterested(aggressor, victim)) { //|| !Papyrus::GetValue<bool>("bMidCombatAssault")) {
					Defeat::defeatactor(victim, true);
				} else {
					// TODO: implement struggle call
					// if (settings->strugglies) {
					// if (Games::Struggle(victim, aggressor, 0)) {
					// 	return;
					// }
					// }
					// TODO: check if 3p+ scenes can start here?
					std::vector<RE::Actor*> partners{ aggressor };
					config->createassault(victim, partners);
				}
			}
			break;
		case DefeatResult::Defeat:
			logger::info("Defeating Actor >> Type = Defeat");
			Defeat::defeatactor(victim, true);
			break;
		}
	}

}  // namespace Kudasai