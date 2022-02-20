#pragma once

#include "Kudasai/Combat/Hooks.h"
#include "Kudasai/Defeat.h"

using Configuration = Papyrus::Integration::Configuration;
using Archetype = RE::EffectArchetypes::ArchetypeID;

namespace Hooks
{
	// ========================================== Hook
	void Entry::InstallHook()
	{
		SKSE::AllocTrampoline(1 << 6);
		auto& trampoline = SKSE::GetTrampoline();
		// ==================================================
		REL::Relocation<std::uintptr_t> wh{ REL::ID(37673) };
		_WeaponHit = trampoline.write_call<5>(wh.address() + 0x3C0, WeaponHit);
		logger::info("WeaponhitHook installed");
		// ==================================================
		// << NOTE: Perk Entry is added later, might have to come back to this >>
		REL::Relocation<std::uintptr_t> mh{ REL::ID(33742) };
		_MagicHit = trampoline.write_call<5>(mh.address() + 0x1E8, MagicHit);
		logger::info("MagicHit Hook installed");
		// ==================================================
		REL::Relocation<std::uintptr_t> ma{ REL::ID(37832) };
		_MagicApply = trampoline.write_call<5>(ma.address() + 0x1E8, MagicApply); // isghost for magic hits
		logger::info("MagicApply Hook installed");
		// ==================================================
		REL::Relocation<std::uintptr_t> det{ REL::ID(41659) };
		_DoDetect = trampoline.write_call<5>(det.address() + 0x526, DoDetect);
		logger::info("Detection Hook installed");


		logger::info("All Hooks installed");
	}  // InstallHook()

	void Entry::WeaponHit(RE::Actor* a_target, RE::HitData* a_hitData)
	{
		// TODO: Test if this can be registered for ded actors
		// auto config = Configuration::GetSingleton()->getsettings();
		if (/*config->active &&*/ a_target && a_hitData) {
			if (Kudasai::Defeat::isdefeated(a_target))
				return;
			const auto aggressor = a_hitData->aggressor.get();
			if (aggressor && a_target != aggressor.get() && !a_target->IsCommandedActor()) {
				logger::info("=== Weaponhit! ========");
				logger::info("victim = {} ;; aggressor = {}", a_target->GetFormID(), aggressor->GetFormID());
				float dmg = a_hitData->totalDamage;
				bool resisted = (static_cast<int>(a_hitData->flags) & (1 + 2 + 4)) == 0;
				auto worns = Kudasai::GetWornArmor(a_target);
				float hp = a_target->GetActorValue(RE::ActorValue::kHealth);
				switch (getDefeated(a_target, aggressor.get(), worns, resisted, dmg)) {
				case HitResult::Proceed:
					validatestrip(a_target, worns);
					break;
				case HitResult::Lethal:
					// negate all av modifiers to stop currently ticking dots..
					a_target->DispelEffectsWithArchetype(Archetype::kValueModifier, false);
					a_target->DispelEffectsWithArchetype(Archetype::kPeakValueModifier, false);
					a_target->DispelEffectsWithArchetype(Archetype::kDualValueModifier, false);
					if (hp < 6)
						a_hitData->totalDamage = 0;
					else
						a_hitData->totalDamage = hp - 2;
					[[fallthrough]];
				default:
					// TODO: defeat here
					
					break;
				}
				logger::info("=======================");
			}
		}
		return _WeaponHit(a_target, a_hitData);
	}  // WeaponHit()

	uint8_t Entry::MagicHit(RE::MagicTarget* a_target, RE::MagicTarget::CreationData* a_data)
	{
		// TODO: Test if this can be registered for ded actors
		// const auto config = Configuration::GetSingleton()->getsettings();
		const auto cf = a_data ? a_data->caster : nullptr;
		if (/*!config->active || */a_target ? (!a_target->MagicTargetIsActor()) : true || cf ? (!cf->Is(RE::FormType::ActorCharacter)) : true)
			return _MagicHit(a_target, a_data);

		const auto target = static_cast<RE::Actor*>(a_target->GetTargetStatsObject());
		const auto caster = static_cast<RE::Actor*>(a_data->caster);
		if (target != caster && !target->IsCommandedActor()) {
			auto& bdata = a_data->effect->baseEffect->data;
			// only allow spells flagged as "kDetrimental" (1 << 2) and not "kRecover" (1 << 1)
			if ((bdata.flags.underlying() & (4 + 2)) == 4) {
				auto at = bdata.archetype;
				// only allow health damaging spells
				if (at == Archetype::kValueModifier || at == Archetype::kPeakValueModifier || at == Archetype::kDualValueModifier) {
					if (bdata.primaryAV == RE::ActorValue::kHealth || bdata.secondaryAV == RE::ActorValue::kHealth) {
						auto efi = &a_data->effect->effectItem;
						logger::info("=== Spellhit! =========");
						logger::info("target = {} ;; caster = {}", target->GetFormID(), caster->GetFormID());
						float magnitude = efi->magnitude;
						float taperdmg = (magnitude * bdata.taperWeight * bdata.taperDuration / (bdata.taperCurve + 1));
						bool resisted = false;	// (static_cast<int>(a_hitData->flags) & (1 + 2 + 4)) == 0;
						auto worns = Kudasai::GetWornArmor(target);
						auto hp = target->GetActorValue(RE::ActorValue::kHealth);
						switch (getDefeated(target, caster, worns, resisted, magnitude + taperdmg)) {
						case HitResult::Proceed:
							validatestrip(target, worns);
							break;
						default:  // HitResult Defeated or Lethal
							// TODO: defeat here

							[[fallthrough]];
						case HitResult::Lethal:
							// negate all av modifiers to stop currently ticking dots..
							a_target->DispelEffectsWithArchetype(Archetype::kValueModifier, false);
							a_target->DispelEffectsWithArchetype(Archetype::kPeakValueModifier, false);
							a_target->DispelEffectsWithArchetype(Archetype::kDualValueModifier, false);
							if (hp < 6)
								efi->magnitude = 0;
							else
								efi->magnitude = hp - 2;
							auto result = _MagicHit(a_target, a_data);
							// taper damage isnt negated here, just restore the damage itll do & reset modified magnitude
							target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, taperdmg);
							efi->magnitude = magnitude;
							return result;
						}
						logger::info("=======================");
					}
				}
			}
		}
		return _MagicHit(a_target, a_data);
	}  // MagicHit()

	bool Entry::MagicApply(RE::Actor* target, RE::MagicTarget* magictarget)
	{
		if(target) {
			logger::info("Target >> {}", target->GetFormID());
			if (Kudasai::Defeat::isdefeated(target))
				return false;
		}
		return _MagicApply(target, magictarget);
	}

	void Entry::DoDetect(RE::Actor* viewer, RE::Actor* target, int& detect, uint8_t* unk04, uint8_t* unk05, int* unk06, RE::NiPoint3* area, float& unk08, float& unk09, float& unk10)
	{
		// logger::info("Do detect");
		if (viewer && target && (Kudasai::Defeat::isdefeated(viewer) || Kudasai::Defeat::isdefeated(target))) {
			// logger::info("Disabling detect");
			return;
		}
		return _DoDetect(viewer, target, detect, unk04, unk05, unk06, area, unk08, unk09, unk10);
	}

	Entry::HitResult Entry::getDefeated(RE::Actor* a_victim, RE::Actor* a_aggressor, std::vector<RE::TESObjectARMO*> wornarmor, bool resisted, float incdmg)
	{
		if (!ValidPair(a_victim, a_aggressor))
			return HitResult::Proceed;

		// const auto config = Configuration::GetSingleton()->getsettings();
		float hp = a_victim->GetActorValue(RE::ActorValue::kHealth);
		bool lethal = hp < incdmg;
		if (resisted && !lethal) {
			return HitResult::Proceed;
		} else if (lethal) {
			// TODO: lay out settings more precisely
			// int flag = (1 << 18) + (1 << 19);  // 18 = essential, 19 = protected
			// if ((a_victim->boolFlags.underlying() & flag) > 0)
			// 	return HitResult::Lethal;
			// else if (config->preventdeath == 1 << 3 && a_victim->IsPlayerRef() || config->preventdeath == 1 << 2 && Kudasai::randomint(0, 99) < config->preventdchance)
				return HitResult::Lethal;
		} else {
			// TODO: rework exposed algorithm
			// if (/* exposed? */ wornarmor.size() < config->armorthresh ||
			// 	/* exhausted stamina? */ Kudasai::getavpercent(a_victim, RE::ActorValue::kStamina) < config->staminathresh ||
			// 	/* exhausted magicka? */ Kudasai::getavpercent(a_victim, RE::ActorValue::kMagicka) < config->magickathresh)
			// 	return HitResult::Defeat;
		}
		return HitResult::Proceed;
	}

	// this checks for..
	// Alreay defeated? || Valid Race? || Excluded? || summoned?
	bool Entry::ValidContender(RE::Actor* a_actor)
	{
		return a_actor != nullptr;
	}  // ValidContender()

	// this checks for..
	// aggressor interested? < Arousal? || Gender? > || Distance? || Hostile?
	bool Entry::ValidPair(RE::Actor* a_victim, RE::Actor* a_aggressor)
	{
		return ValidContender(a_victim) && ValidContender(a_aggressor);
	}  // Valid Pair

	void Entry::validatestrip(RE::Actor*, std::vector<RE::TESObjectARMO*>)
	{
	// 	const auto settings = Configuration::GetSingleton();
	// 	const auto config = settings->getsettings();
	// 	if (a_gearlist.size() && Kudasai::randomint(0, 99) < config->stripchance) {
	// 		const auto em = RE::ActorEquipManager::GetSingleton();

	// 		auto item = a_gearlist.at(Kudasai::randomint(0, static_cast<int>(a_gearlist.size())));
	// 		em->UnequipObject(a_target, item, nullptr, 1, nullptr, true, false, false, true);

	// 		RE::ITEM_REMOVE_REASON reason;
	// 		if (Kudasai::randomint(0, 99) < config->strpchdstry && settings->isstripprotec(item)) {
	// 			reason = RE::ITEM_REMOVE_REASON::kRemove;
	// 			if (a_target->IsPlayerRef()) {
	// 				Kudasai::DebugNotify(item->GetFullName(), " got teared off and destroyed");
	// 			} else {
	// 				if (config->strpdrop)
	// 					reason = RE::ITEM_REMOVE_REASON::kDropping;
	// 				else
	// 					return;
	// 			}
	// 			a_target->RemoveItem(item, 1, reason, nullptr, nullptr);
	// 		}
	// 	}
	}
}  // namespace Hooks
