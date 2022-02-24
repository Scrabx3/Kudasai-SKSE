#include "Kudasai/Combat/Hooks.h"
#include "Kudasai/Combat/Zone.h"
#include "Kudasai/Defeat.h"

using Configuration = Papyrus::Configuration;
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
		// ==================================================
		// << NOTE: Perk Entry is added later, might have to come back to this >>
		REL::Relocation<std::uintptr_t> mh{ REL::ID(33742) };
		_MagicHit = trampoline.write_call<5>(mh.address() + 0x1E8, MagicHit);
		// ==================================================
		// REL::Relocation<std::uintptr_t> ma{ REL::ID(37832) };
		// _MagicApply = trampoline.write_call<5>(ma.address() + 0x3B, MagicApply); // isghost for magic hits
		// ==================================================
		REL::Relocation<std::uintptr_t> det{ REL::ID(41659) };
		_DoDetect = trampoline.write_call<5>(det.address() + 0x526, DoDetect);

		logger::info("Hooks installed");
	}  // InstallHook()

	void Entry::WeaponHit(RE::Actor* a_target, RE::HitData& a_hitData)
	{
		// auto config = Configuration::GetSingleton()->getsettings();
		const auto aggressor = a_hitData.aggressor.get();
		if (/*config->active &&*/ a_target && aggressor && aggressor.get() != a_target && !a_target->IsCommandedActor()) {
			logger::info("Weaponhit -> victim = {} ;; aggressor = {}", a_target->GetFormID(), aggressor->GetFormID());
			if (Kudasai::Defeat::isdefeated(a_target)) {
				logger::info("Victim is defeated");
				return;
			} else if (Papyrus::GetProperty<bool>("bEnabled")) {
				float dmg = a_hitData.totalDamage;
				bool resisted = (static_cast<int>(a_hitData.flags) & (1 + 2 + 4)) == 0;
				auto worns = Kudasai::GetWornArmor(a_target);
				float hp = a_target->GetActorValue(RE::ActorValue::kHealth);
				auto t = getDefeated(a_target, aggressor.get(), worns, resisted, dmg);
				if (t != HitResult::Proceed) {
					if (Kudasai::Zone::GetSingleton()->registerdefeat(a_target, aggressor.get())) {
						if (t == HitResult::Lethal) {
							// negate all av modifiers to stop currently ticking dots..
							a_target->DispelEffectsWithArchetype(Archetype::kValueModifier, false);
							a_target->DispelEffectsWithArchetype(Archetype::kPeakValueModifier, false);
							a_target->DispelEffectsWithArchetype(Archetype::kDualValueModifier, false);
							if (hp < 6)
								a_hitData.totalDamage = 0;
							else
								a_hitData.totalDamage = hp - 2;
						}
					} else {
						logger::info("Failed to register defeat, abandon");
						validatestrip(a_target, worns);
					}
				}
			}
		}
		return _WeaponHit(a_target, a_hitData);
	}  // WeaponHit()

	uint8_t Entry::MagicHit(RE::MagicTarget* a_target, RE::MagicTarget::CreationData* a_data)
	{
		const auto casterREF = a_data ? a_data->caster : nullptr;
		if ((casterREF ? !(casterREF->Is(RE::FormType::ActorCharacter)) : true) || (a_target ? !(a_target->MagicTargetIsActor()) : true))
			return _MagicHit(a_target, a_data);
		
		const auto target = static_cast<RE::Actor*>(a_target->GetTargetStatsObject());
		const auto caster = static_cast<RE::Actor*>(casterREF);
		if (target != caster && !target->IsCommandedActor()) {
			auto& bdata = a_data->effect->baseEffect->data;
			if ((bdata.primaryAV == RE::ActorValue::kHealth || bdata.secondaryAV == RE::ActorValue::kHealth) && ((bdata.archetype == Archetype::kValueModifier || bdata.archetype == Archetype::kPeakValueModifier || bdata.archetype == Archetype::kDualValueModifier))) {
				auto flags = bdata.flags.underlying() & (4 + 2);  // Detremential + Recover
				if (flags == 4) {
					bool blockdamage = false;
					logger::info("Spellhit -> target = {} ;; caster = {}", target->GetFormID(), caster->GetFormID());
					auto efi = &a_data->effect->effectItem;
					const float magnitude = efi->magnitude;
					const float taperdmg = (magnitude * bdata.taperWeight * bdata.taperDuration / (bdata.taperCurve + 1));
					if (Kudasai::Defeat::isdefeated(target)) {
						logger::info("Victim is defeated");
						blockdamage = true;
					} else if (Papyrus::GetProperty<bool>("bEnabled")) {
						auto worns = Kudasai::GetWornArmor(target);
						const bool resisted = false;
						const auto t = getDefeated(target, caster, worns, resisted, magnitude + taperdmg);
						if (t != HitResult::Proceed) {
							if (Kudasai::Zone::GetSingleton()->registerdefeat(target, caster))
								blockdamage = t == HitResult::Lethal;
							else {
								logger::info("Failed to register defeat, abandon");
								validatestrip(target, worns);
							}
						}
					}
					if (blockdamage) {
						// negate all av modifiers to stop currently ticking dots..
						a_target->DispelEffectsWithArchetype(Archetype::kValueModifier, false);
						a_target->DispelEffectsWithArchetype(Archetype::kPeakValueModifier, false);
						a_target->DispelEffectsWithArchetype(Archetype::kDualValueModifier, false);
						auto hp = target->GetActorValue(RE::ActorValue::kHealth);
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
				} else if (Kudasai::Defeat::isdefeated(target)) {
					if (flags == 0)
						Kudasai::Defeat::restoreactor(target, true);
					// TODO: check if this spell would heal the target
				}
			}
		}
		return _MagicHit(a_target, a_data);
	}  // MagicHit()

	uint8_t* Entry::DoDetect(RE::Actor* viewer, RE::Actor* target, int32_t& detectval, uint8_t& unk04, uint8_t& unk05, uint32_t& unk06, RE::NiPoint3& pos, float& unk08, float& unk09, float& unk10)
	{
		if (viewer && target && (Kudasai::Defeat::isdefeated(viewer) || Kudasai::Defeat::isdefeated(target))) {
			detectval = -1000;
			return nullptr;
		}
		return _DoDetect(viewer, target, detectval, unk04, unk05, unk06, pos, unk08, unk09, unk10);
	}

	Entry::HitResult Entry::getDefeated(RE::Actor* a_victim, RE::Actor* a_aggressor, std::vector<RE::TESObjectARMO*> wornarmor, bool resisted, float incdmg)
	{
		if (!ValidPair(a_victim, a_aggressor))
			return HitResult::Proceed;

		// const auto config = Configuration::GetSingleton()->getsettings();
		float hp = a_victim->GetActorValue(RE::ActorValue::kHealth);
		bool lethal = hp < incdmg;
		if (resisted && !lethal) {
			logger::info("<getdefeated> incomming hit was resisted and isnt lethal");
			return HitResult::Proceed;
		} else if (lethal) {
			// TODO: lay out settings more precisely
			// int flag = (1 << 18) + (1 << 19);  // 18 = essential, 19 = protected
			// if ((a_victim->boolFlags.underlying() & flag) > 0)
			// 	return HitResult::Lethal;
			// else if (config->preventdeath == 1 << 3 && a_victim->IsPlayerRef() || config->preventdeath == 1 << 2 && Kudasai::randomint(0, 99) < config->preventdchance)
			logger::info("<getdefeated> incomming hit is lethal");
			return HitResult::Lethal;
		} else {
			// TODO: rework exposed algorithm
			// if (/* exposed? */ wornarmor.size() < config->armorthresh ||
			// 	/* exhausted stamina? */ Kudasai::getavpercent(a_victim, RE::ActorValue::kStamina) < config->staminathresh ||
			// 	/* exhausted magicka? */ Kudasai::getavpercent(a_victim, RE::ActorValue::kMagicka) < config->magickathresh)
			// 	return HitResult::Defeat;
		}
		logger::info("<getdefeated> incomming hit has no valid scenario");
		return HitResult::Proceed;
	}

	bool Entry::ValidPair(RE::Actor* a_victim, RE::Actor* a_aggressor)
	{
		if (!a_victim->IsHostileToActor(a_aggressor))
			return false;
		auto d = a_victim->GetPosition().GetDistance(a_aggressor->GetPosition());
		logger::info("Distance {} to {} is {}", a_victim->GetFormID(), a_aggressor->GetFormID(), d);
		if (d > 4096.0f)
			return false;
		return ValidContender(a_victim) && ValidContender(a_aggressor);
	}

	bool Entry::ValidContender(RE::Actor* a_actor)
	{
		if (a_actor->IsDead())
			return false;
		return !Configuration::GetSingleton()->isexcludedactor(a_actor);
	}

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
