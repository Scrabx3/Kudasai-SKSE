#include "Kudasai/Combat/Zone.h"

#include "Kudasai/Combat/Resolution.h"
#include "Kudasai/Defeat.h"
#include "Papyrus/Settings.h"

namespace Kudasai
{
	namespace Config = Papyrus::Configuration;

	bool Zone::registerdefeat(RE::Actor* victim, RE::Actor* aggressor)
	{
		logger::info("Aggressor = {} -> Register Defeat for Victim = {}", aggressor->GetFormID(), victim->GetFormID());
		// victim cant be commanded, hit evaluation already checks that
		if (aggressor->IsCommandedActor()) {
			auto tmp = aggressor->GetCommandingActor().get();
			if (tmp) {
				logger::info("Aggressor = {} is summon, using Summoner = {} as Aggressor", aggressor->GetFormID(), tmp->GetFormID());
				aggressor = tmp.get();
			} else {
				logger::warn("Aggressor = {} is summon but no Summoner found? Abandon", aggressor->GetFormID());
				return false;
			}
		}
		if (aggressor->IsPlayerRef()) {
			std::thread(&Zone::defeat, victim, aggressor, DefeatResult::Defeat).detach();
			return true;
		}

		auto dtype = getdefeattype(aggressor);
		if (dtype == DefeatResult::Cancel)
			return false;
		std::thread(&Zone::defeat, victim, aggressor, dtype).detach();
		return true;
	}

	Zone::DefeatResult Zone::getdefeattype(RE::Actor* aggressor)
	{
		using Res = Zone::DefeatResult;

		auto agrzone = aggressor->GetCombatGroup();
		if (!agrzone || agrzone->members.empty()) {
			logger::warn("Aggressor = {} has no Combat Group, Abandon", aggressor->GetFormID());
			return Res::Cancel;
		}
		const auto validtarget = [](const RE::ActorPtr t) -> bool {
			return t && !t->IsCommandedActor() && t->Is3DLoaded() && !t->IsDead() && !Defeat::IsDamageImmune(t.get());
		};

		std::set<RE::FormID> targets;
		for (auto& e : agrzone->targets) {
			const auto& target = e.targetHandle.get();
			if (!target)
				continue;
			else if (validtarget(target))
				targets.insert(target->GetFormID());

			if (const auto& combatgroup = target->GetCombatGroup(); combatgroup) {
				for (const auto& member : combatgroup->members) {
					if (const auto t = member.memberHandle.get(); validtarget(t))
						targets.insert(t->GetFormID());
				}
			}
		}
		logger::info("Aggressor = {} has targets = {}", aggressor->GetFormID(), targets.size());
		switch (targets.size()) {
		case 0:
			return Res::Cancel;
		case 1:
			return Res::Resolution;
		default:
			return Res::Defeat;
		}
	}

	void Zone::defeat(RE::Actor* victim, RE::Actor* aggressor, DefeatResult result)
	{
		// delay to make the player be defeated 'after' the hit
		std::this_thread::sleep_for(std::chrono::microseconds(650));
		const auto pd = PlayerDefeat::GetSingleton();
		const auto pdactive = pd->Active;
		std::scoped_lock lock(_m);
		if (victim->IsDead() || victim->IsInKillMove() || Defeat::isdefeated(victim)) {
			logger::info("{} -> Victim is dead, defeated or in killmove", victim->GetFormID());
			if (result == DefeatResult::Resolution)
				victim = nullptr;
			else
				return;
		} else {
			SKSE::GetTaskInterface()->AddTask([victim]() {
				const auto process = victim->currentProcess;
				const auto middlehigh = process ? process->middleHigh : nullptr;
				if (middlehigh) {
					for (auto& commandedActorData : middlehigh->commandedActors) {
						const auto summon = commandedActorData.activeEffect;
						if (summon)
							summon->Dispel(true);
					}
				}
			});
			if (!victim->IsPlayerRef()) {
				if (Papyrus::GetSetting<bool>("bNotifyDefeat")) {
					std::string msg;
					if (Papyrus::GetSetting<bool>("bNotifyColored")) {
						auto color = Papyrus::GetSetting<RE::BSFixedString>("sNotifyColorChoice");
						msg = fmt::format("<font color = '{}'>{} has been defeated by {}</font color>", color, victim->GetDisplayFullName(), aggressor->GetDisplayFullName());
					} else {
						msg = fmt::format("{} has been defeated by {}", victim->GetDisplayFullName(), aggressor->GetDisplayFullName());
					}
					RE::DebugNotification(msg.c_str());
				}
			}
		}

		SKSE::GetTaskInterface()->AddTask([=]() {
			switch (result) {
			case DefeatResult::Resolution:
				if (victim)
					Defeat::defeat(victim);

				if (Serialize::GetSingleton()->Defeated.contains(0x14)) {
					if (pdactive && !pd->Active) {
						// This is true only if _m has been locked by the pd and started a quest/rescued the Player
						// In such case, do not start quest/rescue here
						return;
					}
					PlayerDefeat::Unregister();
					if (!CreatePlayerResolution(aggressor, false)) {
						std::thread([]() {
							const auto player = RE::PlayerCharacter::GetSingleton();
							std::this_thread::sleep_for(std::chrono::seconds(6));
							SKSE::GetTaskInterface()->AddTask([=]() {
								Defeat::rescue(player, false);
							});
							std::this_thread::sleep_for(std::chrono::seconds(4));
							Defeat::undopacify(player);
						}).detach();
					}
				} else if (!aggressor->IsPlayerTeammate()) {	 // followers do not start the resolution quest
					CreateNPCResolution(aggressor);
				}
				break;
			case DefeatResult::Defeat:
				Defeat::defeat(victim);
				if (victim->IsPlayerRef()) {
					if (Random::draw<float>(0, 99.5) < Papyrus::GetSetting<float>("fMidCombatBlackout"))
						if (CreatePlayerResolution(aggressor, true))
							break;
					PlayerDefeat::Register();
				}
				break;
			}
		});
	}

	bool Zone::CreatePlayerResolution(RE::Actor* aggressor, bool blackout)
	{
		using rType = Resolution::Type;

		const auto player = RE::PlayerCharacter::GetSingleton();
		const auto processLists = RE::ProcessLists::GetSingleton();
		const auto GuardDiaFac = RE::TESForm::LookupByID<RE::TESFaction>(0x0002BE3B);
		const auto isguard = [&GuardDiaFac](RE::ActorPtr ptr) {
			return ptr->IsGuard() && ptr->IsInFaction(GuardDiaFac);
		};
		auto type = [&]() {
			if (aggressor->IsPlayerTeammate())
				return rType ::Follower;
			else if (aggressor->IsHostileToActor(player))
				return rType::Hostile;
			else
				return rType::Neutral;
		}();
		// Collect Team Members of this Aggressor (Followers if Aggressor is Pl. Teammate)
		std::vector<RE::Actor*> memberlist{ aggressor };
		memberlist.reserve(processLists->highActorHandles.size() / 4);
		for (auto& e : processLists->highActorHandles) {
			auto ptr = e.get();
			if (!ptr || ptr->IsDead() || Defeat::IsDamageImmune(ptr.get()) || ptr->IsHostileToActor(aggressor))
				continue;

			const bool hostile = ptr->IsHostileToActor(player);
			if (type != rType::Guard && !blackout && hostile && isguard(ptr)) {
				memberlist = std::vector{ ptr.get() };
				type = rType::Guard;
				continue;
			}
			if (ptr.get() == aggressor)
				continue;

			switch (type) {
			case rType::Follower:
				if (ptr->IsPlayerTeammate())
					memberlist.emplace_back(ptr.get());
				break;
			case rType::Hostile:
				if (hostile)
					memberlist.emplace_back(ptr.get());
				break;
			case rType::Neutral:
				if (!hostile)
					memberlist.emplace_back(ptr.get());
				break;
			case rType::Guard:
				if (isguard(ptr))
					memberlist.emplace_back(ptr.get());
			}
		}
		// List fully build. Request a Post Combat Quest & start it
		auto q = Resolution::GetSingleton()->SelectQuest(type, memberlist, blackout);
		return q && q->Start();
	}

	void Zone::CreateNPCResolution(RE::Actor* aggressor)
	{
		const auto handler = RE::TESDataHandler::GetSingleton();
		const auto npcresQ = handler->LookupForm<RE::TESQuest>(QuestNPCResolution, ESPNAME);
		if (!npcresQ)
			return;
		if (!npcresQ->IsStopped()) {
			logger::warn("NPC Resolution already running");
			return;
		}
		const auto cg = aggressor->GetCombatGroup();
		if (!cg)
			return;

		std::vector<std::pair<RE::Actor*, std::vector<RE::Actor*>>> viclist;
		{
			std::set<RE::Actor*> agrlist;
			for (auto& e : cg->members)	 // populate lists
				if (auto ptr = e.memberHandle.get(); ptr)
					agrlist.insert(ptr.get());

			auto& Defeated = Serialize::GetSingleton()->Defeated;
			for (auto it = Defeated.begin(); it != Defeated.end();) {
				auto actor = RE::TESForm::LookupByID<RE::Actor>(*it);
				if (!actor || actor->IsDead())
					it = Defeated.erase(it);
				else {
					if (actor->Is3DLoaded() && Config::IsValidRace(actor)) {
						if (!actor->IsHostileToActor(aggressor))
							agrlist.insert(actor);
						else if (viclist.size() < 15)
							viclist.push_back(std::make_pair(actor, std::vector<RE::Actor*>()));
					}
					it++;
				}
			}
			if (viclist.empty())
				return;

			const auto GetDistance = [](RE::Actor* prim, RE::Actor* sec) -> float {
				return prim->GetPosition().GetDistance(sec->GetPosition());
			};
			// assign every aggressor to a victim closest to them
			for (auto& victoire : agrlist) {
				if (Random::draw<int>(0, 99) < 10) {  // oddity an aggressor will ignore the scene
					continue;
				}
				for (auto& [defeated, victoires] : viclist) {
					const auto distance = GetDistance(defeated, victoire);
					if (distance < 1500.0f && Config::IsInterested(defeated, victoire)) {  // interested?
						for (auto& [key, value] : viclist) {							   // already in a previous list?
							if (key == defeated) {										   // no previous list
								victoires.push_back(victoire);
								break;
							}
							auto where = std::find(value.begin(), value.end(), victoire);
							if (where != value.end()) {	 // in previous list
								// assume victoire will switch targets only if new target is closer + 30% chance
								if (distance < GetDistance(key, victoire) && Random::draw<int>(0, 99) < 30) {
									value.erase(where);
									value.push_back(victoire);
								}
								break;	// victoire can only be in at most one previous vector
							}
						}
					}
				}
			}
		}
		// at this point, viclist is a map with <= 15 elements, mapping victims to a vector of aggressors interested in it
		// the quest holds at most 50 victoires. Remove any lone victims and any group that would break the 50 Actor limit
		size_t total = 0;
		for (auto entry = viclist.begin(); entry != viclist.end();) {
			if (entry->second.empty()) {
				entry = viclist.erase(entry);
			} else {
				if (total += entry->second.size(); total >= 50) {
					viclist.erase(entry, viclist.end());
					break;
				}
				entry++;
			}
		}
		if (viclist.empty())
			return;

		const std::vector<RE::BGSKeyword*> links{
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution00, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution01, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution02, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution03, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution04, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution05, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution06, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution07, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution08, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution09, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution10, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution11, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution12, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution13, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(KeywordNPCResolution14, ESPNAME)
		};
		const auto tmpfriends = handler->LookupForm<RE::TESFaction>(FactionTmpFriends, ESPNAME);

		int i = 0;
		for (auto& [victim, list] : viclist) {
			for (auto& victoire : list)
				victoire->extraList.SetLinkedRef(victim, links[i]);
			victim->AddToFaction(tmpfriends, 0);
			i++;
		}

		if (!npcresQ->Start()) {
			logger::warn("Failed to start NPC Resolution");
			int n = 0;
			for (auto& [victim, list] : viclist) {
				for (auto& victoire : list)
					victoire->extraList.SetLinkedRef(nullptr, links[n]);
				RemoveFromFaction(victim, tmpfriends);
				n++;
			}
		}
	}

	void Zone::PlayerDefeat::Register()
	{
		auto me = GetSingleton();
		me->Active = true;
		std::thread(&Cycle, me).detach();
	}

	void Zone::PlayerDefeat::Unregister()
	{
		GetSingleton()->Active = false;
	}

	void Zone::PlayerDefeat::Cycle()
	{
		const auto main = RE::Main::GetSingleton();
		const auto player = RE::PlayerCharacter::GetSingleton();
		const auto processLists = RE::ProcessLists::GetSingleton();
		// ----------------------- while { ...
Reset:
		std::this_thread::sleep_for(std::chrono::seconds(2));
		if (!main->gameActive)
			goto Reset;
		if (!Defeat::isdefeated(player) || !Active)
			return;
		RE::Actor* doquest = nullptr;
		for (auto& e : processLists->highActorHandles) {
			auto ptr = e.get();
			if (ptr == nullptr)
				continue;
			if (ptr->IsHostileToActor(player) && ptr->GetPosition().GetDistance(player->GetPosition()) < 6144.0f) {
				if (ptr->IsInCombat())
					goto Reset;
				doquest = ptr.get();
			}
		}
		// ----------------------- }
		std::scoped_lock lock(_m);
		if (!Active)
			return;
		Active = false;
		if (doquest)
			if (auto q = Resolution::GetSingleton()->SelectQuest(Resolution::Type::Hostile, std::vector{ doquest }, false); q && q->Start())
				return;
		SKSE::GetTaskInterface()->AddTask([]() {
			Defeat::rescue(RE::PlayerCharacter::GetSingleton(), true);
		});
	}

}  // namespace Kudasai
