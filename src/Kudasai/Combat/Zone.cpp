#include "Kudasai/Combat/Zone.h"

#include "Kudasai/Defeat.h"
#include "Kudasai/Struggle/Struggly.h"
#include "Papyrus/Settings.h"

namespace Kudasai
{
	bool Zone::registerdefeat(RE::Actor* victim, RE::Actor* aggressor)
	{
		logger::info("<registerdefeat> Victim = {} <<->> Aggressor = {}", victim->GetFormID(), aggressor->GetFormID());
		if (aggressor->IsPlayerRef())
			std::thread(&Zone::defeat, victim, aggressor, DefeatResult::Defeat).detach();

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
				// // TODO: implement
				// logger::info("Defeating Actor >> Type = Resolution");
				// // check if the player is a victim here before doing anything fancy
				// auto& defeats = Srl::GetSingleton()->defeats;
				// auto pldefeat = defeats.find(0x14) != defeats.end();
				// constexpr auto fallback = []() {
				// 	std::this_thread::sleep_for(std::chrono::seconds(4));
				// 	auto pl = RE::PlayerCharacter::GetSingleton();
				// 	Defeat::rescue(pl, true);
				// };

				// if (victim->IsPlayerRef() || pldefeat) {
				// 	if (aggressor->IsPlayerTeammate()) {
				// 		// if player is down & last standin is a follower, let the follower help up the player
				// 		// IDEA: allow support for custom rescue quests?
				// 		auto handler = RE::TESDataHandler::GetSingleton();
				// 		auto q = handler->LookupForm<RE::TESQuest>(0x808E87, ESPNAME);
				// 		if (!q->Start()) {
				// 			// if Quest couldnt start manually put player out of defeat
				// 			std::thread(fallback).detach();
				// 		}
				// 	} else if (!aggressor->IsHostileToActor(victim))
				// 		// if last standing is not a follower but not hostile anyway, let the player leave silently
				// 		std::thread(fallback).detach();
				// 	} else {
				// 		// TODO: match victoires with victims for Scenes (or just ignore..)
				// 	}
				}
			break;
		case DefeatResult::Assault:
			{
				using Config = Papyrus::Configuration;
				logger::info("Defeating Actor >> Type = Assault");

				if (!Papyrus::GetProperty<bool>("bMidCombatAssault"))
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
					if ((Config::isnpc(actor) || Config::isvalidcreature(actor)) && Config::isinterested(victim, { actor })) {
						victoire = actor;
						break;
					}
				}
				if (!victoire)
					return;
				// create Struggle/Assault -> Defeat as Fallback :<
				try {
					Struggle::PlayStruggle(victim, victoire);
					std::this_thread::sleep_for(std::chrono::milliseconds(2500)); // animation lean in

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
								Animation::ExitPaired(victim, victoire, { "bleedoutStart", "IdleForceDefautlState" });
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

					Defeat::pacify(victim);
					Defeat::pacify(aggressor);
				} catch (const std::exception& e) {
					logger::warn("Error Starting Struggle for Victim = {} -> Error = {}", victim->GetFormID(), e.what());

					if (Config::createassault(victim, { victoire })) {
						Defeat::pacify(victim);
						Defeat::setdamageimmune(victim, true);
						Defeat::pacify(victoire);
						Defeat::setdamageimmune(victoire, true);
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