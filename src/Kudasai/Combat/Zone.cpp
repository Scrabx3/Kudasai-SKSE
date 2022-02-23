#pragma once

#include "Kudasai/Combat/Zone.h"
#include "Kudasai/Defeat.h"
#include "Kudasai/Game.h"

namespace Kudasai
{
	bool Zone::registerdefeat(RE::Actor* victim, RE::Actor* aggressor)
	{
		logger::info("<registerdefeat> Victim = {} <<->> Aggressor = {}", victim->GetFormID(), aggressor->GetFormID());
		auto agrzone = aggressor->GetCombatGroup();
		auto viczone = victim->GetCombatGroup();
		if (!agrzone || !viczone) {
			logger::warn("Failed to register defeat, associated Combat Group missing >> agr = {} >> vic = {}", agrzone == nullptr, viczone == nullptr);
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
		auto dtype = getdefeattype(agrzone);
		if (dtype == DefeatResult::Cancel)
			return false;
		_m.lock();
		// if not cancel, dispell all of the defeated actors summons
		const auto process = victim->currentProcess;
		const auto middlehigh = process ? process->middleHigh : nullptr;
		if (middlehigh) {
			for (auto& data : middlehigh->commandedActors) {
				const auto eff = data.activeEffect;
				if (eff)
					victim->InvalidateCommandedActorEffect(eff);
			}
		}
		switch (dtype) {
		case DefeatResult::Resolution:
			std::thread(&Zone::resolution, this, victim, aggressor).detach();
			break;
		case DefeatResult::Assault:
			std::thread(&Zone::assault, this, victim, aggressor).detach();
			break;
		case DefeatResult::Defeat:
			std::thread(&Zone::defeat, this, victim).detach();
			break;
		}
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
			auto ptr = target.targetHandle.get();
			if (valid(ptr))
				ret++;
		}
		return ret;
	}
	int Zone::countvalid(RE::BSTArray<RE::CombatGroup::MemberData> list)
	{
		int ret = 0;
		for (auto& target : list) {
			auto ptr = target.handle.get();
			if (valid(ptr))
				ret++;
		}
		return ret;
	}

	void Zone::resolution(RE::Actor*, RE::Actor*)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(300));
		logger::warn("Nop, no resolution yet");
		// TODO: implement. Important to think about a way to properly gather all defeated actors & assign them the specific faction
		_m.unlock();
	}

	void Zone::assault(RE::Actor* victim, RE::Actor* aggressor)
	{
		logger::info("Assault ->> victim {} >> aggressor = {}", victim->GetFormID(), aggressor->GetFormID());
		std::this_thread::sleep_for(std::chrono::microseconds(300));
		if (victim->IsDead() || Defeat::isdefeated(victim) || victim->IsInKillMove()) {
			_m.unlock();
			return;
		}
		/* 
			if adult Scenes are disabled or the aggressor shows no interested, defeat the victim
			else if struggle enabled play struggle & wait for return
			else assault the victim
		*/
		auto config = Papyrus::Integration::Configuration::GetSingleton();
		auto settings = config->getsettings();	// 1 << Mid Combat << Post Combat << Always << Never
		if (aggressor->IsDead() || Defeat::isdefeated(aggressor) || !config->isactorinterested(aggressor, victim) || (settings->scenario & (1 << 1) + (1 << 2)) == 0) {
			Defeat::defeatactor(victim, true);
		} else {
			// interested and permitted, start struggle or create assault
			if (settings->strugglies) {
				// if (Games::Struggle(victim, aggressor, 0)) {
				// 	_m.unlock();
				// 	return;
				// }
			}
			// TODO: check if 3p+ scenes can start here?
			std::vector<RE::Actor*> partners{ aggressor };
			config->createassault(victim, partners);
		}
		_m.unlock();
	}

	void Zone::defeat(RE::Actor* victim)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(300));
		if ((victim->IsDead() + Defeat::isdefeated(victim) + victim->IsInKillMove()) == 0)
			Defeat::defeatactor(victim, true);
		_m.unlock();
	}


	// // update zone
	// auto zone = findzone(victim);
	// if (!zone) {
	// 	auto cg = victim->GetCombatGroup();
	// 	if (!cg)
	// 		return false;
	// 	for (auto& member : cg->members) {
	// 		auto subj = member.handle.get();
	// 		if (subj)
	// 			zone = findzone(subj.get());
	// 		if (zone)
	// 			break;
	// 	}
	// }
	// if (!zone) {
	// 	try {
	// 		zone = &createzone(victim);
	// 	} catch (const std::exception&) {
	// 		logger::critical("Failed to create zone, abandon registration");
	// 		return false;
	// 	}
	// } else {
	// 	try {
	// 		zone->ZoneUpdate();
	// 	} catch (const std::exception&) {
	// 		logger::critical("Failed to update zone, abandon registration");
	// 		return false;
	// 	}
	// }
	// // consider that the Victim or Aggressor may be not be active at this point in time
	// if (aggressor->IsCommandedActor()) {
	// 	auto tmp = aggressor->GetCommandingActor().get();
	// 	if (tmp)
	// 		aggressor = tmp.get();
	// 	else
	// 		return false;
	// }
	// auto vicdata = zone->getmemberdata(victim);
	// auto agrdata = zone->getmemberdata(aggressor);
	// if (vicdata->status != agrdata->status || vicdata->status != Zone::MemberData::Status::Active)
	// 	return false;
	// switch (zone->allowassault(aggressor)) {
	// case Zone::ZoneStatus::Cancel:
	// 	return false;
	// default:
	// 	{
	// 		// kill all summons controlled by the victim
	// 		const auto process = victim->currentProcess;
	// 		const auto middlehigh = process ? process->middleHigh : nullptr;
	// 		if (middlehigh) {
	// 			for (auto& data : middlehigh->commandedActors) {
	// 				const auto eff = data.activeEffect;
	// 				if (eff)
	// 					victim->InvalidateCommandedActorEffect(eff);
	// 			}
	// 		}
	// 	}

	// 	__fallthrough;
	// case Zone::ZoneStatus::Assault:
	// 	// std::thread(&Zone::assault, zone, victim, aggressor);
	// 	break;
	// case Zone::ZoneStatus::Resolution:
	// 	// std::thread(&Zone::resolution, zone, victim, aggressor);
	// 	break;
	// }
	// return true;
	// }

	// 	Zone& ZoneFactory::createzone(RE::Actor* subject)
	// 	{
	// 		try {
	// 			return zones.emplace_back(subject);
	// 		} catch (const std::exception& e) {
	// 			throw e;
	// 		}
	// 	}

	// 	Zone* ZoneFactory::findzone(RE::Actor* subject)
	// 	{
	// 		for (auto& zone : zones) {
	// 			if (zone.allianceof(subject) != nullptr)
	// 				return &zone;
	// 		}
	// 		return nullptr;
	// 	}

	// 	// ========================================================== Zone

	// 	Zone::Zone(RE::Actor* prim)
	// 	{
	// 		logger::info("Creating new Zone with prim {} ( {} )", prim->GetFormID(), prim->GetName());
	// 		Alliance premier;
	// 		auto cg = prim->GetCombatGroup();
	// 		if (!cg) {
	// 			logger::critical("Object {} does not own a CombatGroup", prim->GetFormID());
	// 			throw null_zone();
	// 		}
	// 		for (auto& member : cg->members) {
	// 			auto ally = member.handle.get();
	// 			if (ally)
	// 				premier.emplace(ally->GetFormID(), std::pair(ally, MemberData(ally.get())));
	// 		}
	// 		alliances.push_back(premier);
	// 		// TODO: Remove below info after conformation
	// 		logger::info("root is part of cg -> {}", premier.contains(prim->GetFormID()));
	// 		for (auto& target : cg->targets) {
	// 			auto enemy = target.targetHandle.get();
	// 			if (!enemy)
	// 				continue;
	// 			// if this actor is not part of an alliance yet, they arent allied with any previous actors and thus actor of a new, isolated alliance
	// 			if (allianceof(enemy.get()) == nullptr) {
	// 				Alliance secundair;
	// 				cg = enemy->GetCombatGroup();
	// 				if (!cg)
	// 					continue;
	// 				for (auto& member : cg->members) {
	// 					auto hostile = member.handle.get();
	// 					if (hostile)
	// 						secundair.emplace(hostile->GetFormID(), std::pair(hostile, MemberData(hostile.get())));
	// 				}
	// 				alliances.push_back(secundair);
	// 			}
	// 		}
	// 	}

	// 	void Zone::ZoneUpdate()
	// 	{
	// 		auto somepremier = alliances[0];
	// 		RE::CombatGroup* cg;
	// 		for (auto& prim : somepremier) {
	// 			cg = prim.second.first->GetCombatGroup();
	// 			if (cg)
	// 				break;
	// 		}
	// 		if (!cg) {
	// 			logger::critical("<ZoneUpdate> No Combatgroup found");
	// 			throw null_zone();
	// 		}
	// 		for (auto& member : cg->members) {
	// 			auto ally = member.handle.get();
	// 			if (ally) {
	// 				int ID = ally->GetFormID();
	// 				if (alliances[0].contains(ID)) {
	// 					if (ally->IsDead() || !ally->Is3DLoaded())
	// 						alliances[0].erase(ID);
	// 				} else
	// 					alliances[0].emplace(ID, std::pair(ally, MemberData(ally.get())));
	// 			}
	// 		}
	// 		for (auto& target : cg->targets) {
	// 			auto enemy = target.targetHandle.get();
	// 			if (enemy) {
	// 				cg = enemy->GetCombatGroup();
	// 				if (!cg)
	// 					continue;
	// 				Alliance* mine = allianceof(enemy.get());
	// 				if (!mine) {
	// 					// newly joined the find. Either one of its allies is part of an alliance or this is an entirely new faction in need of its own alliance
	// 					for (auto& member : cg->members) {
	// 						auto subject = member.handle.get();
	// 						if (subject) {
	// 							mine = allianceof(subject.get());
	// 							if (mine != nullptr)
	// 								break;
	// 						}
	// 					}
	// 				}
	// 				if (!mine) {
	// 					Alliance alliance;
	// 					for (auto& member : cg->members) {
	// 						auto ally = member.handle.get();
	// 						if (ally)
	// 							alliance.emplace(ally->GetFormID(), std::pair(ally, MemberData(ally.get())));
	// 					}
	// 					alliances.push_back(alliance);
	// 				} else {
	// 					for (auto& member : cg->members) {
	// 						auto ally = member.handle.get();
	// 						if (ally) {
	// 							int ID = ally->GetFormID();
	// 							if (mine->contains(ID)) {
	// 								if (ally->IsDead() || !ally->Is3DLoaded())
	// 									mine->erase(ID);
	// 							} else
	// 								mine->emplace(ID, std::pair(ally, MemberData(ally.get())));
	// 						}
	// 					}
	// 				}
	// 			}
	// 		}
	// 	}

	// 	Alliance* Zone::allianceof(RE::Actor* actor)
	// 	{
	// 		logger::info("Checking Alliance for {}", actor->GetFormID());
	// 		for (auto& al : alliances) {
	// 			if (al.find(actor->GetFormID()) != al.end()) {
	// 				return &al;
	// 			}
	// 		}
	// 		return nullptr;
	// 	}

	// 	Zone::MemberData* Zone::getmemberdata(RE::Actor* subject)
	// 	{
	// 		auto alliance = allianceof(subject);
	// 		if (alliance) {
	// 			auto data = alliance->find(subject->GetFormID())->second;
	// 			return &data.second;
	// 		}
	// 		return nullptr;
	// 	}

	// 	Zone::ZoneStatus Zone::allowassault(RE::Actor* aggressor)
	// 	{
	// 		/* Defining < Victim .versus. Aggressor >
	// 		Assuming the fight is not over after this knockdown
	// 		X v 1 -> No assault
	// 		X v Y -> Allow assault if Y >= X/2

	// 		Assuming the fight is over after this knockdown
	// 		X v Y -> Begin Resolution
	// 		*/
	// 		using Status = Zone::ZoneStatus;
	// 		int k = aggressor->GetFormID();
	// 		int agrsize = 0;
	// 		int enemysize = 0;
	// 		for (int i = 0; i < alliances.size(); i++) {
	// 			Alliance alliance = alliances[i];
	// 			if (alliance.contains(k)) {
	// 				for (auto& member : alliance) {
	// 					if (member.second.second.status == MemberData::Status::Active)
	// 						agrsize++;
	// 				}
	// 			} else {
	// 				for (auto& member : alliance) {
	// 					if (member.second.second.status == MemberData::Status::Active)
	// 						enemysize++;
	// 				}
	// 			}
	// 		}
	// 		if (enemysize == 0)
	// 			return Status::Cancel;
	// 		else if (enemysize == 1)  // one enemy left standing, assuming this to be the current victim
	// 			return Status::Resolution;
	// 		else if ((agrsize * 2) > enemysize)	 // only allow assault if there are at most twice the amount of enemies
	// 			return Status::Assault;
	// 		return Status::Cancel;
	// 	}

	// 	void Zone::assault(RE::Actor* victim, RE::Actor* aggressor)
	// 	{
	// 		logger::info("Assault ->> victim {} >> aggressor = {}", victim->GetFormID(), aggressor->GetFormID());
	// 		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	// 		auto m = std::scoped_lock();
	// 		// in case something went wrong during damage calculation
	// 		if (victim->IsDead() || aggressor->IsDead())
	// 			return;
	// 		using Status = MemberData::Status;
	// 		auto vicdata = getmemberdata(victim);
	// 		auto agrdata = getmemberdata(aggressor);
	// 		if (vicdata->status != agrdata->status || vicdata->status != Status::Active)
	// 			return;
	// 		vicdata->status = Status::Processing;
	// 		agrdata->status = Status::Processing;
	// 		/* what happens here..
	// 			pull the victim & aggressor out of combat (and make sure they stay out of combat)

	// 			if adult Scenes are disabled or the aggressor shows no interested, defeat the victim
	// 			else if struggle enabled play struggle & wait for return
	// 			else assault the victim
	// 		*/
	// 		auto config = Papyrus::Integration::Configuration::GetSingleton();
	// 		auto settings = config->getsettings();
	// 		// 1 << Mid Combat << Post Combat << Always << Never
	// 		if ((settings->scenario & (1 << 1) + (1 << 2)) > 0 && config->isactorinterested(aggressor, victim)) {
	// 			// interested, struggle minigame or straight to assault
	// 			if (settings->strugglies) {
	// 				// TODO: implement struggling mechanic here
	// 				// Games::Struggle(victim, aggressor, this);
	// 				return;
	// 			}
	// 			// TODO: check if 3p+ scenes can start here?
	// 			std::vector<RE::Actor*> partners{ aggressor };
	// 			config->createassault(victim, partners);
	// 		} else {
	// 			// not interested (or disabled), jump straight to the defeat
	// 			defeatactor(victim);
	// 			agrdata->status = Status::Active;
	// 		}
	// 	}

	// 	void Zone::resolution(RE::Actor* victim, RE::Actor* aggressor)
	// 	{
	// 		logger::info("Resolution ->> victim {} >> aggressor = {}", victim->GetFormID(), aggressor->GetFormID());
	// 		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	// 		std::scoped_lock();
	// 		// TODO: Create post combat instance
	// 		// it is essential to ensure that the actor is alive & able to be defeated

	// 		// (config->scenario & (1 << 0) + (1 << 2)) > 0;
	// 	}

	// 	void Zone::graceperiod(RE::Actor* subject, MemberData* data)
	// 	{
	// 		data->status = MemberData::Status::Grace;
	// 		float restore = static_cast<float>(subject->GetPermanentActorValue(RE::ActorValue::kHealth) * 0.2);
	// 		subject->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, restore);

	// 		// TODO: remove bleedout here(?)

	// 		std::this_thread::sleep_for(std::chrono::seconds(3));
	// 		data->status = MemberData::Status::Active;
	// 	}

}  // namespace Kudasai
