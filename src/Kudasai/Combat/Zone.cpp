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
			return Res::Assault;
			// return Res::Defeat;
		} else if (agrnum * 2 <= tarnum) {
			logger::info("Aggressor is outmatched; returning type Defeat");
			// return Res::Defeat;
			return Res::Assault;
		}
		logger::info("Returning type Assault");
		return Res::Assault;
		// return Res::Defeat;
		// return Res::Resolution;
	}
	int Zone::countvalid(RE::BSTArray<RE::CombatGroup::TargetData>& list)
	{
		int ret = 0;
		for (auto& target : list) {
			auto that = target.targetHandle.get();
			if (that && !that->IsCommandedActor() && !Defeat::isdefeated(that.get()))
				ret++;
		}
		return ret;
	}
	int Zone::countvalid(RE::BSTArray<RE::CombatGroup::MemberData>& list)
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
				logger::info("Defeating Actor >> Type = Assault");
				// check all near aggressors if they are potential intrests for this victim
				using Config = Papyrus::Configuration;
				std::vector<RE::Actor*> list{};
				const auto validvictoire = [&](RE::Actor* victoire) {
					if (victoire->GetPosition().GetDistance(victim->GetPosition()) > 512)
						return false;
					if (list.size() > 0) {
						const auto race = list[0]->GetRace();
						return Config::isnpc(list[0]) ? Config::isnpc(victoire) : victoire->GetRace() == race;
					}
					return true;
				};
				auto cg = aggressor->GetCombatGroup();
				if (cg) {
					int agrnum = countvalid(cg->members);
					int tarnum = countvalid(cg->targets);
					int available = agrnum - tarnum;  // agrnum is at least twice of target num
					if (available > 4)				  // allowing partners 4 at most
						available = 4;
					logger::info("Allowed Actors = {}", available);
					for (auto& victoiredata : cg->members) {
						auto victoire = victoiredata.handle.get();
						if (victoire && validvictoire(victoire.get())) {
							list.push_back(victoire.get());
							logger::info("Adding Actor to Assault List = {}", victoire->GetFormID());
							if (--available == 0)
								break;
						}
					}
				} else if (validvictoire(aggressor)) {
					logger::info("Adding Actor to Assault List = {}", aggressor->GetFormID());
					list.push_back(aggressor);
				}
				if (aggressor->IsDead() || Defeat::isdefeated(aggressor) || !Papyrus::GetProperty<bool>("bMidCombatAssault") ||
					list.size() == 0 || !Config::isinterested(victim, list)) {
					Defeat::defeat(victim);
				} else {
					// TODO: implement struggle call
					// if (settings->strugglies) {
					// if (Games::Struggle(victim, aggressor, 0)) {
					// 	return;
					// }
					// }
					if (Config::createassault(victim, list)) {
						Defeat::pacify(victim);
						Defeat::setdamageimmune(victim, true);
						std::for_each(list.begin(), list.end(), [](RE::Actor* subject) { Defeat::pacify(subject); Defeat::setdamageimmune(subject, true); });
					} else
						Defeat::defeat(victim);
				}
			}
			break;
		case DefeatResult::Defeat:
			logger::info("Defeating Actor >> Type = Defeat");
			Defeat::defeat(victim);
			break;
		}
	}

}  // namespace Kudasai