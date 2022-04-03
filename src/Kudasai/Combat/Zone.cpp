#include "Kudasai/Combat/Zone.h"

#include "Kudasai/Defeat.h"
#include "Kudasai/Struggle/Struggly.h"
#include "Papyrus/Settings.h"

namespace Kudasai
{
	using Config = Papyrus::Configuration;

	bool Zone::registerdefeat(RE::Actor* victim, RE::Actor* aggressor)
	{
		logger::info("<registerdefeat> Victim = {} <<->> Aggressor = {}", victim->GetFormID(), aggressor->GetFormID());
		if (aggressor->IsPlayerRef()) {
			std::thread(&Zone::defeat, victim, aggressor, DefeatResult::Assault).detach();
			return true;
		}

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
		auto dtype = getdefeattype(agrzone);
		if (dtype == DefeatResult::Cancel)
			return false;
		std::thread(&Zone::defeat, victim, aggressor, dtype).detach();
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
		return Res::Assault
			;
		/*
		logger::info("agrnum = {} <<>> tarnum = {}", agrnum, tarnum);
		if (tarnum == 1) {
			logger::info("Target is final victim; returning type Resolution");
			return Res::Resolution;
		} else if (agrnum * 2 <= tarnum) {
			logger::info("Aggressor is outmatched; returning type Defeat");
			return Res::Defeat;
		}
		logger::info("Returning type Assault");
		return Res::Assault;
		*/
	}

	void Zone::defeat(RE::Actor* victim, RE::Actor* aggressor, DefeatResult result)
	{
		// delay to make the player be defeated 'after' the hit
		std::this_thread::sleep_for(std::chrono::microseconds(450));

		std::scoped_lock();
		if (victim->IsDead() || victim->IsInKillMove() || Defeat::isdefeated(victim) || Struggle::FindPair(victim) != nullptr) {
			const bool combatend = result == DefeatResult::Resolution;
			logger::info("Victim = {} is dead, defeated or in killmove. Defeat is Combat End = {}", victim->GetFormID(), combatend);
			if (combatend)
				victim = nullptr;
			else
				return;
		} else {
			const auto process = victim->currentProcess;
			const auto middlehigh = process ? process->middleHigh : nullptr;
			if (middlehigh) {
				for (auto& data : middlehigh->commandedActors) {
					const auto eff = data.activeEffect;
					if (eff)
						victim->InvalidateCommandedActorEffect(eff);
				}
			}
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

		switch (result) {
		case DefeatResult::Resolution:
			{
				logger::info("Defeating Actor >> Type = Resolution");
				if (victim)
					Defeat::defeat(victim);

				constexpr auto fallback = []() {
					auto pl = RE::PlayerCharacter::GetSingleton();
					std::this_thread::sleep_for(std::chrono::seconds(6));
					Defeat::rescue(pl, false);
					std::this_thread::sleep_for(std::chrono::seconds(3));
					Defeat::undopacify(pl);
				};

				auto& defeats = Srl::GetSingleton()->defeats;
				if (defeats.find(0x00000014) != defeats.end()) {
					auto task = SKSE::GetTaskInterface();
					task->AddTask([aggressor]() {
						auto handler = RE::TESDataHandler::GetSingleton();
						if (aggressor->IsPlayerTeammate()) {
							logger::info("Player -> Follower Rescue");
							// IDEA: allow support for custom rescue quests?
							
							auto q = handler->LookupForm<RE::TESQuest>(0x808E87, ESPNAME);	// default follower help player
							if (!q->Start())
								fallback();

						} else if (!aggressor->IsHostileToActor(RE::PlayerCharacter::GetSingleton())) {
							logger::info("Player -> Enemy isnt hostile. Pulling Player out of Defeat");
							fallback();
							
						} else {
							logger::info("Player -> Default Resolution");
							// IDEA: allow support for custom resolution quests?
							// This is the fallback Quest that simply ports the player to some random location
							auto q = handler->LookupForm<RE::TESQuest>(0x88C931, ESPNAME);
							if (!q->Start())
								fallback();
						}
					});
				} else if (!aggressor->IsPlayerTeammate() && Papyrus::GetSetting<bool>("bPostCombatAssault")) {	 // followers do not start the resolution quest
					CreateNPCResolution(aggressor);
				}
			}
			break;
		case DefeatResult::Assault:
			{
				logger::info("Defeating Actor >> Type = Assault");
				if (randomREAL<float>(0, 99.5) < Papyrus::GetSetting<float>("fMidCombatBlackout")) {
					logger::info("Blackout Trigger");
					// TODO: create Blackout here
					// return;
				} else if (!Papyrus::GetSetting<bool>("bMidCombatAssault")) {
					logger::info("Mid Combat Assaults are disabled");
					Defeat::defeat(victim);
					return;
				}

				// collect all nearby actors that are hostile to the victim..
				std::vector<RE::Actor*> neighbours;
				const auto validneighbour = [&victim](RE::Actor* victoire) {
					if (victoire->IsDead() || Defeat::isdefeated(victoire))
						return false;
					return victoire->GetPosition().GetDistance(victim->GetPosition()) <= 512;
				};
				auto cg = aggressor->GetCombatGroup();
				if (cg) {
					for (auto& victoiredata : cg->members) {
						auto victoire = victoiredata.handle.get();
						if (victoire && validneighbour(victoire.get()))
							neighbours.push_back(victoire.get());
					}
				} else if (validneighbour(aggressor)) {
					neighbours.push_back(aggressor);
				}
				if (neighbours.size() == 0) {
					Defeat::defeat(victim);
					return;
				}
				// from all neighbours, look for one thats interested
				RE::Actor* victoire = nullptr;
				for (auto it = neighbours.begin(); true; it++) {
					if (it == neighbours.end())
						return;
					
					const auto actor = *it;
					if (Config::isvalidrace(actor) && Config::isinterested(victim, { actor })) {
						victoire = actor;
						break;
					}
				}

				// create Struggle/Assault -> Defeat as Fallback :<
				try {
					Defeat::pacify(victim);
					Defeat::pacify(victoire);

					Struggle::StruggleType type;
					double difficulty;
					if (!victim->IsPlayerRef()) {
						type = Struggle::StruggleType::None;
						// for NONE, set Base Escape Chance to 50%
						// lose 2.5% Chance for every Neighbour after the first
						// Lose % equal to Hp missing beyond 50%
						auto sizepenalty = 2.5 * (neighbours.size() - 1);
						auto hpp = getavpercent(victim, RE::ActorValue::kHealth);
						auto healthpenalty = hpp < 0.5 ? (1 - hpp) * 100 : 0;
						difficulty = 50 - sizepenalty - healthpenalty;
					} else {
						type = Struggle::StruggleType::QTE;
						// for QTE, set Base Timing at 2.7
						// Lose 7% for every Neighbour after the first
						// Lose 2% for every Hp missing beyond 50%
						constexpr double base = 2.7;
						auto sizepenalty = (0.07 * base) * (neighbours.size() - 1);
						auto hpp = getavpercent(victim, RE::ActorValue::kHealth);
						auto healthpenalty = hpp < 0.5 ? (1 - hpp) * 100 * (0.02 * base) : 0;
						difficulty = base - sizepenalty - healthpenalty;
					}

					auto struggle = new Struggle([victim, victoire](const bool victory, Struggle* struggle) {
						logger::info("Zone Callback -> Victim = {} / Victoire = {}, Victory = {}", victim->GetFormID(), victoire->GetFormID(), victory);
						if (victory) {	// victim won -> make sure it has at least 30% Hp & get both back int othe fight
							struggle->PlayBreakfree();

							auto Health = RE::ActorValue::kHealth;
							float tempAV = victim->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, Health);
							float totalAV = victim->GetPermanentActorValue(Health) + tempAV;
							float currentAV = victim->GetActorValue(Health);
							if (totalAV / currentAV < 0.3) {
								float thirty = totalAV * 0.3f;
								victim->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, Health, thirty - currentAV);
							}

							Defeat::undopacify(victoire);
							std::thread([victim]() {  // grace period before the Victim is forced back into Combat
								std::this_thread::sleep_for(std::chrono::seconds(4));
								Defeat::undopacify(victim);
							})
								.detach();
						} else {  // victim lost -> create an assault or (if failed) force bleedout
							Animation::ExitPaired(victim, victoire, { "bleedoutStart"s, "IdleForceDefaultState"s });
							if (Config::createassault(victim, { victoire })) {
								Defeat::setdamageimmune(victim, true);
								Defeat::setdamageimmune(victoire, true);
							} else {
								Defeat::undopacify(victoire);
								Defeat::defeat(victim);
							}
						}

						// sometimes executed on the mainthread which causes the game to freeze zz
						std::thread([struggle]() { delete struggle; }).detach();
						return;
					},
						victim, aggressor);
					struggle->BeginStruggle(difficulty, type);

				} catch (const std::exception& e) {
					logger::warn("Error Starting Struggle for Victim = {} -> Error = {}", victim->GetFormID(), e.what());

					if (Config::createassault(victim, { victoire })) {
						Defeat::setdamageimmune(victim, true);
						Defeat::setdamageimmune(victoire, true);
					} else {
						Defeat::undopacify(victoire);
						Defeat::defeat(victim);
					}
				}
			}
			break;
		case DefeatResult::Defeat:
			logger::info("Defeating Actor >> Type = Defeat");
			Defeat::defeat(victim);
			break;
		}
	}

	void Zone::CreateNPCResolution(RE::Actor* aggressor)
	{
		logger::info("NPC -> NPC Resolution");
		auto cg = aggressor->GetCombatGroup();
		if (!cg) {
			logger::warn("No Combat Group for Aggressor, aborting");
			return;
		}

		std::set<RE::Actor*> agrlist;
		std::map<RE::Actor*, std::vector<RE::Actor*>> viclist;
		
		// populate lists
		for (auto& member : cg->members) {
			auto ptr = member.handle.get();
			if (ptr)
				agrlist.insert(ptr.get());
		}
		auto& defeats = Srl::GetSingleton()->defeats;
		for (auto it = defeats.begin(); it != defeats.end();) {
			auto actor = RE::TESForm::LookupByID<RE::Actor>(*it);
			if (!actor || actor->IsDead())
				it = defeats.erase(it);
			else {
				if (actor->Is3DLoaded() && Config::isvalidrace(actor)) {
					if (!actor->IsHostileToActor(aggressor))
						agrlist.insert(actor);
					else if (viclist.size() < 15)
						viclist.insert(std::make_pair(actor, std::vector<RE::Actor*>()));
				}
				it++;
			}
		}
		if (viclist.empty()) {
			logger::info("No Victims in Area? Aborting");
			return;
		}

		const auto GetDistance = [](RE::Actor* prim, RE::Actor* sec) -> float {
			return prim->GetPosition().GetDistance(sec->GetPosition());
		};
		// assign every aggressor to a victim closest to them
		for (auto& victoire : agrlist) {
			if (randomINT<short>(0, 99) < 20) {	 // oddity an aggressor will ignore the scene
				continue;
			}
			for (auto& [defeated, victoires] : viclist) {
				const auto distance = GetDistance(defeated, victoire);
				if (distance < 750.0f && Config::isinterested(defeated, { victoire })) {  // interested?
					for (auto& [key, value] : viclist) {								  // already in a previous list?
						if (key == defeated) {											  // no previous list
							victoires.push_back(victoire);
							break;
						}
						auto where = std::find(value.begin(), value.end(), victoire);
						if (where != value.end()) {	 // in previous list
							// assume victoire will switch targets only if new target is closer + 30% chance
							if (distance < GetDistance(key, victoire) && randomINT<short>(0, 99) < 30) {
								value.erase(where);
								value.push_back(victoire);
							} else {
								victoires.push_back(victoire);
							}
							break;	// victoire can only be in at most one previous vector
						}
					}
				}
			}
		}

		// at this point, viclist is a map with <= 15 elements, mapping victims to a vector of aggressors interested in it
		// the quest holds at most 50 victoires. Remove any lone victims and any group that would break the 50 Actor limit
		size_t total = 0;
		for (auto entry = viclist.begin(); entry != viclist.end();) {
			const auto size = entry->second.size();
			if (entry->second.empty() || total + size > 50) {
				entry = viclist.erase(entry);
			} else {
				total += size;
				if (size == 50)
					break;
				entry++;
			}				
		}

		const auto handler = RE::TESDataHandler::GetSingleton();
		const auto npcresQ = handler->LookupForm<RE::TESQuest>(0x8130AF, ESPNAME);
		if (!npcresQ->IsStopped()) {
			logger::warn("NPC Resolution already running");
			return;
		}
		const std::vector<RE::BGSKeyword*> links{
			handler->LookupForm<RE::BGSKeyword>(0x81309F, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x80DF8D, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x813093, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x813094, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x813095, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x813096, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x813097, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x813098, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x813099, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x81309A, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x81309B, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x81309C, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x81309D, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x81309E, ESPNAME),
			handler->LookupForm<RE::BGSKeyword>(0x8130A0, ESPNAME)
		};

		auto task = SKSE::GetTaskInterface();
		task->AddTask([npcresQ, viclist, links]() {			
			int i = 0;
			for (auto& pair : viclist) {
				for (auto& victoire : pair.second)
					victoire->extraList.SetLinkedRef(pair.first, links[i]);
				i++;
			}

			if (!npcresQ->Start()) {
				logger::warn("Failed to start NPC Resolution");
				int n = 0;
				for (auto& pair : viclist) {
					for (auto& victoire : pair.second)
						victoire->extraList.SetLinkedRef(nullptr, links[n]);
					n++;
				}
			}
		});
	}

}  // namespace Kudasai
