#include "Kudasai/Combat/Zone.h"

#include "Kudasai/Defeat.h"
#include "Kudasai/Struggle/Struggly.h"
#include "Papyrus/Settings.h"

namespace Kudasai
{
	bool Zone::registerdefeat(RE::Actor* victim, RE::Actor* aggressor)
	{
		logger::info("<registerdefeat> Victim = {} <<->> Aggressor = {}", victim->GetFormID(), aggressor->GetFormID());
		if (aggressor->IsPlayerRef()) {
			// TODO: result is defeat
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

	void Zone::defeat(RE::Actor* victim, RE::Actor* aggressor, DefeatResult result)
	{
		using Config = Papyrus::Configuration;
		constexpr auto validate = [](RE::Actor* vic, RE::Actor* agr) {
			return (Config::isnpc(agr) || Config::isvalidcreature(agr)) && Config::isinterested(vic, { agr });
		};

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
			{
				// TODO: implement
				logger::info("Defeating Actor >> Type = Resolution");
				// check if the player is a victim here before doing anything fancy
				auto& defeats = Srl::GetSingleton()->defeats;
				auto pldefeat = defeats.find(0x14) != defeats.end();
				constexpr auto fallback = []() {
					std::this_thread::sleep_for(std::chrono::seconds(4));
					auto pl = RE::PlayerCharacter::GetSingleton();
					Defeat::rescue(pl, true);
				};

				if (victim->IsPlayerRef() || pldefeat) {
					if (aggressor->IsPlayerTeammate()) {
						// IDEA: allow support for custom rescue quests?
						auto handler = RE::TESDataHandler::GetSingleton();
						auto q = handler->LookupForm<RE::TESQuest>(0x808E87, ESPNAME);	// default follower help player
						if (!q->Start())
							std::thread(fallback).detach();
					} else if (!aggressor->IsHostileToActor(RE::PlayerCharacter::GetSingleton())) {
						// last standing isnt an enemy to the player, just let the player go
						std::thread(fallback).detach();
					} else {
						// TODO: default resolution
					}
				} else {
					auto cg = aggressor->GetCombatGroup();
					if (!cg) {
						// incomplete cg -> just defeat and have nothing happen zz
						Defeat::defeat(victim);
						return;
					}

					std::vector<RE::Actor*> agrlist;
					std::map<RE::Actor*, std::vector<RE::Actor*>> viclist {std::make_pair(victim, std::vector<RE::Actor*>())};
					for (auto& member : cg->members) {
						agrlist.push_back(member.handle.get().get());
					}
					auto storage = Srl::GetSingleton()->defeats;
					for (auto& defeated : storage) {
						auto actor = RE::TESForm::LookupByID<RE::Actor>(defeated);
						if (!actor || actor->IsDead()) {
							logger::warn("Defeated {} does not exist or is dead", defeated);
							storage.erase(defeated);
							continue;
						}
						if (!actor->Is3DLoaded()) {
							logger::info("Defeated {} has no 3D loaded", defeated);
							continue;
						}
						if (actor->IsHostileToActor(aggressor) && viclist.size() < 15)
							viclist.insert(std::make_pair(actor, std::vector<RE::Actor*>()));
						else
							agrlist.push_back(actor);
					}

					constexpr auto GetDistance = [](RE::Actor* prim, RE::Actor* sec) {
						return prim->GetPosition().GetDistance(sec->GetPosition());
					};
					// assign every aggressor to a victim closest to them
					for (size_t i = agrlist.size() - 1; !agrlist.empty(); i--) {
						if (randomINT<short>(0, 99) < 20) {	 // oddity an aggressor will ignore the scene
							agrlist.pop_back();
							continue;
						}
						auto victoire = agrlist[i];
						for (const auto& [defeated, victoires] : viclist) {
							auto d = GetDistance(defeated, victoire);
							if (d < 750.0f && validate(defeated, victoire)) {  // already assigned?
								for (auto& [key, value] : viclist) {
									if (key == defeated) {
										value.push_back(victoire);
										break;
									}

									auto where = std::find(value.begin(), value.end(), victoire);
									if (where != value.end()) {
										// victoire will "change their mind" if the current victim is closer (& a 30% RNG check)
										auto d2 = GetDistance(key, victoire);
										if (d < d2 && randomINT<short>(0, 99) < 30) {
											value.erase(where);
											value.push_back(victoire);
										}
										break;
									}
								}
							}
						}
						agrlist.pop_back();
					}
					// // at this point, agrlist is empty and viclist is a map with <= 15 elements, mapping victims to a vector of aggressors interested in it
					// // the resolution quest for npc holds a total of 50 aliases which can divide between up to 15 victims
					size_t total = 0;
					for (auto entry = viclist.begin(); entry != viclist.end();) {
						total += entry->second.size();
						if (total > 50) {
							viclist.erase(entry, viclist.end());
							break;
						}
						if (entry->second.empty())
							entry = viclist.erase(entry);
						else
							entry++;
					}

					const auto handler = RE::TESDataHandler::GetSingleton();
					const auto npcresQ = handler->LookupForm<RE::TESQuest>(0x80DF8C, ESPNAME);
					if (npcresQ->IsRunning())
						return;

					static const auto links = []() {
						const auto handler = RE::TESDataHandler::GetSingleton();
						std::vector<RE::BGSKeyword*> ret;
						ret[0] = handler->LookupForm<RE::BGSKeyword>(0x81309F, ESPNAME);
						ret[1] = handler->LookupForm<RE::BGSKeyword>(0x80DF8D, ESPNAME);
						ret[2] = handler->LookupForm<RE::BGSKeyword>(0x813093, ESPNAME);
						ret[3] = handler->LookupForm<RE::BGSKeyword>(0x813094, ESPNAME);
						ret[4] = handler->LookupForm<RE::BGSKeyword>(0x813095, ESPNAME);
						ret[5] = handler->LookupForm<RE::BGSKeyword>(0x813096, ESPNAME);
						ret[6] = handler->LookupForm<RE::BGSKeyword>(0x813097, ESPNAME);
						ret[7] = handler->LookupForm<RE::BGSKeyword>(0x813098, ESPNAME);
						ret[8] = handler->LookupForm<RE::BGSKeyword>(0x813099, ESPNAME);
						ret[9] = handler->LookupForm<RE::BGSKeyword>(0x81309A, ESPNAME);
						ret[10] = handler->LookupForm<RE::BGSKeyword>(0x81309B, ESPNAME);
						ret[11] = handler->LookupForm<RE::BGSKeyword>(0x81309C, ESPNAME);
						ret[12] = handler->LookupForm<RE::BGSKeyword>(0x81309D, ESPNAME);
						ret[13] = handler->LookupForm<RE::BGSKeyword>(0x81309E, ESPNAME);
						ret[14] = handler->LookupForm<RE::BGSKeyword>(0x8130A0, ESPNAME);
						for (auto&& kw : ret) {
							logger::info("Keyword ID = {}", kw->GetFormID());
						}
						return ret;
					}();

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
				}
			}
			break;
		case DefeatResult::Assault:
			{
				logger::info("Defeating Actor >> Type = Assault");

				if (!Papyrus::GetSetting<bool>("bMidCombatAssault"))
					Defeat::defeat(victim);
				// collect all nearby actors that are hostile to the victim..
				std::vector<RE::Actor*> neighbours;
				const auto validneighbour = [&](RE::Actor* victoire) {
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
				for (auto& actor : neighbours) {
					if (validate(victim, actor)) {
						victoire = actor;
						break;
					}
				}
				if (!victoire)
					return;
				// create Struggle/Assault -> Defeat as Fallback :<
				try {
					Struggle::PlayStruggle(victim, victoire);
					std::this_thread::sleep_for(std::chrono::milliseconds(2500));  // animation lean in

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
					Defeat::pacify(victim);
					Defeat::pacify(victoire);

					Struggle::BeginStruggle([victim, victoire](bool victory) {
						logger::info("Zone Callback -> Victory = {}", victory);

						if (victory) {
							// Victim won, so we just have it stand back up heal it some and give it another try to win the fight or so
							// The Victoire is send straight back to combat
							Struggle::PlayBreakfree(victim, victoire);
							// restore Hp until 30%
							auto Health = RE::ActorValue::kHealth;
							float tempAV = victim->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, Health);
							float totalAV = victim->GetPermanentActorValue(Health) + tempAV;
							float currentAV = victim->GetActorValue(Health);
							if (totalAV / currentAV < 0.3) {
								float thirty = totalAV * 0.3f;
								victim->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, Health, thirty - currentAV);
							}
							// give Victim a grace period before enemies turn hostile again
							std::thread([victim]() {
								std::this_thread::sleep_for(std::chrono::seconds(4));
								Defeat::undopacify(victim);
							}).detach();
						} else {
							// Victim lost, meaning an assault is expected.. or if that fails just force bleedout
							if (Config::createassault(victim, { victoire })) {
								Animation::ExitPaired(victim, victoire, { "bleedoutStart", "IdleForceDefaultState" });
								Defeat::setdamageimmune(victim, true);
								Defeat::setdamageimmune(victoire, true);
								return;
							} else {
								SetVehicle(victim, nullptr);
								SetVehicle(victoire, nullptr);

								Defeat::defeat(victim);
								Animation::ForceDefault(victoire);

								SetRestrained(victim, false);
								SetRestrained(victoire, false);
							}
						}
						Defeat::undopacify(victoire);
					},
						difficulty, type);

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

}  // namespace Kudasai