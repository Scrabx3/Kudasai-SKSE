#include "Kudasai/Combat/Hooks.h"

#include "Kudasai/Combat/Zone.h"
#include "Kudasai/Defeat.h"
#include "Kudasai/Struggle/Struggly.h"
#include "Papyrus/Settings.h"

using Configuration = Papyrus::Configuration;
using Archetype = RE::EffectArchetypes::ArchetypeID;

namespace Kudasai
{
	// ========================================== Hook
	void Hooks::InstallHook()
	{
		SKSE::AllocTrampoline(1 << 6);
		auto& trampoline = SKSE::GetTrampoline();
		// ==================================================
		REL::Relocation<std::uintptr_t> wh{ RELID(37673, 38627) };
		_WeaponHit = trampoline.write_call<5>(wh.address() + OFFSET(0x3C0, 0x4a8), WeaponHit);
		// ==================================================
		// << NOTE: Perk Entry is added later, might have to come back to this >>
		// << maybe to hook an instance where the ActiveEffect is applied? >>
		REL::Relocation<std::uintptr_t> mh{ RELID(33742, 33742) };
		_MagicHit = trampoline.write_call<5>(mh.address() + OFFSET(0x1E8, 0x1E8), MagicHit);
		// ==================================================
		REL::Relocation<std::uintptr_t> ma{ RELID(37832, 37832) };
		_DoApplyMagic = trampoline.write_call<5>(ma.address() + OFFSET(0x3B, 0x3B), DoApplyMagic);
		// ==================================================
		REL::Relocation<std::uintptr_t> det{ RELID(41659, 41659) };
		_DoDetect = trampoline.write_call<5>(det.address() + OFFSET(0x526, 0x526), DoDetect);

		logger::info("Hooks installed");
	}  // InstallHook()


	void Hooks::WeaponHit(RE::Actor* a_target, RE::HitData& a_hitData)
	{
		const auto aggressor = a_hitData.aggressor.get();
		if (a_target && aggressor && aggressor.get() != a_target && !a_target->IsCommandedActor() && Papyrus::Configuration::isnpc(a_target)) {
			logger::info("Weaponhit -> victim = {} ;; aggressor = {}", a_target->GetFormID(), aggressor->GetFormID());
			if (Kudasai::Defeat::isdefeated(a_target)) {
				return;
			} else if (auto struggle = Struggle::FindPair(a_target); struggle != nullptr) {
				logger::info("Hitting a struggling target..");
				struggle->StopStruggle(a_target);
				return;
			} else if (Papyrus::GetSetting<bool>("bEnabled")) {
				float hp = a_target->GetActorValue(RE::ActorValue::kHealth);
				const float spelldamage = GetIncomingEffectDamage(a_target);
				auto t = GetDefeated(a_target, aggressor.get(), hp < a_hitData.totalDamage + spelldamage);
				if (t != HitResult::Proceed) {
					if (Kudasai::Zone::registerdefeat(a_target, aggressor.get())) {
						if (t == HitResult::Lethal) {
							RemoveDamagingSpells(a_target);
							if (hp < 6)
								a_hitData.totalDamage = 0;
							else
								a_hitData.totalDamage = hp - 2;
						}
					} else {
						ValidateStrip(a_target, false);
					}
				}
			}
		}
		return _WeaponHit(a_target, a_hitData);
	}  // WeaponHit()

	char Hooks::MagicHit(RE::MagicTarget* a_target, RE::MagicTarget::CreationData* a_data)
	{
		const auto casterREF = a_data ? a_data->caster : nullptr;
		if ((casterREF ? !casterREF->Is(RE::FormType::ActorCharacter) : true) || (a_target ? !a_target->MagicTargetIsActor() : true))
			return _MagicHit(a_target, a_data);
		else if (!Papyrus::GetSetting<bool>("bEnabled"))
			return _MagicHit(a_target, a_data);

		const auto target = static_cast<RE::Actor*>(a_target->GetTargetStatsObject());
		const auto caster = static_cast<RE::Actor*>(casterREF);
		if (target != caster && !target->IsCommandedActor() && Papyrus::Configuration::isnpc(target)) {
			auto& effectdata = a_data->effect->baseEffect->data;
			if (SpellModifiesHealth(effectdata, true)) {
				logger::info("Spellhit -> target = {} ;; caster = {}", target->GetFormID(), caster->GetFormID());
				auto efi = &a_data->effect->effectItem;
				const float taperdmg = GetTaperDamage(efi->magnitude, effectdata);
				const auto hp = target->GetActorValue(RE::ActorValue::kHealth);
				const auto t = GetDefeated(target, caster, hp < efi->magnitude + taperdmg);
				if (t != HitResult::Proceed) {
					if (Kudasai::Zone::registerdefeat(target, caster)) {
						Defeat::setdamageimmune(target, true);
						if (t == HitResult::Lethal) {
							RemoveDamagingSpells(target);
							const float magnitude = efi->magnitude;
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
					}
				} else {
					ValidateStrip(target, true);
				}
			}
		}
		return _MagicHit(a_target, a_data);
	}  // MagicHit()

	bool Hooks::DoApplyMagic(RE::Actor* target, RE::MagicItem* item)
	{
		if (!target || !item)
			return _DoApplyMagic(target, item);

		const auto hittype = [&]() {
			for (auto& effect : item->effects) {
				auto base = effect ? effect->baseEffect : nullptr;
				if (!base)
					continue;
				auto& data = base->data;
				if (SpellModifiesHealth(data, false)) {
					auto flak = data.flags.underlying() & (4 + 2);
					if (flak == 0)
						return 1;
					else if (flak == 4)
						return 2;
				}
			}
			return 0;
		};

		if (Defeat::isdefeated(target)) {
			switch (hittype()) {
			case 1:
				logger::info("Defeated Victim = {} has been healed. Curing..", target->GetFormID());
				Defeat::rescue(target, true);
				return true;
			case 2:
				return true;
			}
		} else if (auto struggle = Struggle::FindPair(target); struggle != nullptr) {
			switch (hittype()) {
			case 1:
				{
					logger::info("Struggling Victim = {} has been healed", target->GetFormID());
					auto other = struggle->victim == target ? struggle->aggressor : target;
					struggle->StopStruggle(other);
					return true;
				}
				break;
			case 2:
				logger::info("Struggling Victim = {} has been hit", target->GetFormID());
				struggle->StopStruggle(target);
				return true;
			}
		}

		return _DoApplyMagic(target, item);
	}

	uint8_t* Hooks::DoDetect(RE::Actor* viewer, RE::Actor* target, int32_t& detectval, uint8_t& unk04, uint8_t& unk05, uint32_t& unk06, RE::NiPoint3& pos, float& unk08, float& unk09, float& unk10)
	{
		if (viewer && target && (Kudasai::Defeat::ispacified(viewer) || Kudasai::Defeat::ispacified(target))) {
			detectval = -1000;
			return nullptr;
		}
		return _DoDetect(viewer, target, detectval, unk04, unk05, unk06, pos, unk08, unk09, unk10);
	}

	Hooks::HitResult Hooks::GetDefeated(RE::Actor* a_victim, RE::Actor* a_aggressor, const bool lethal)
	{
		if (!ValidPair(a_victim, a_aggressor))
			return HitResult::Proceed;

		if (lethal) {
			using flak = RE::Actor::BOOL_FLAGS;
			bool protecc;
			bool essential = Papyrus::GetSetting<bool>("bLethalEssential");
			if (essential && (a_victim->boolFlags.all(flak::kEssential) || (!a_aggressor->IsPlayerRef() && a_victim->boolFlags.all(flak::kProtected))))
				protecc = true;
			else if (a_victim->IsPlayerRef())
				protecc = Kudasai::randomREAL<float>(0, 99) < Papyrus::GetSetting<float>("fLethalPlayer");
			else
				protecc = Kudasai::randomREAL<float>(0, 99) < Papyrus::GetSetting<float>("fLethalNPC");
			logger::info("Protecting Actor from lethal hit = {}", protecc);
			return protecc ? HitResult::Lethal : HitResult::Proceed;
		} else {
			// TODO: rework exposed algorithm
			// if (/* exposed? */ wornarmor.size() < config->armorthresh ||
			// 	/* exhausted stamina? */ Kudasai::getavpercent(a_victim, RE::ActorValue::kStamina) < config->staminathresh ||
			// 	/* exhausted magicka? */ Kudasai::getavpercent(a_victim, RE::ActorValue::kMagicka) < config->magickathresh)
			// 	return HitResult::Defeat;
		}
		return HitResult::Proceed;
	}

	const float Hooks::GetTaperDamage(const float magnitude, const RE::EffectSetting::EffectSettingData& data)
	{
		return abs(magnitude * data.taperWeight * data.taperDuration / (data.taperCurve + 1));
	}

	const float Hooks::GetIncomingEffectDamage(RE::Actor* subject)
	{
		const auto effects = subject->GetActiveEffectList();
		if (!effects)
			return 0.0f;

		float ret = 0.0f;
		for (const auto& effect : *effects) {
			if (!effect || effect->flags.all(RE::ActiveEffect::Flag::kDispelled))
				continue;

			if (const auto base = effect->GetBaseObject(); base && SpellModifiesHealth(base->data, true)) {
				const float taper = GetTaperDamage(effect->magnitude, base->data);
				const float remainingtime = (effect->duration - effect->elapsedSeconds) / effect->duration;
				const float incoming = abs(effect->magnitude * remainingtime + taper);
				logger::info("Adding Taper Damage = {} (Taper = {})", incoming, taper);
				ret += incoming;
			}
		}
		return ret;
	}

	void Hooks::RemoveDamagingSpells(RE::Actor* subject)
	{
		auto effects = subject->GetActiveEffectList();
		if (!effects)
			return;

		for (auto& eff : *effects) {
			if (!eff || eff->flags.all(RE::ActiveEffect::Flag::kDispelled))
				continue;
			auto base = eff->GetBaseObject();
			if (base && SpellModifiesHealth(base->data, true)) {
				logger::info("Dispelling Spell = {}", base->GetFormID());
				eff->Dispel(true);
			}
		}
	}

	bool Hooks::SpellModifiesHealth(RE::EffectSetting::EffectSettingData& data, const bool check_damaging)
	{
		if ((data.primaryAV == RE::ActorValue::kHealth || data.secondaryAV == RE::ActorValue::kHealth) &&
			(data.archetype == Archetype::kValueModifier || data.archetype == Archetype::kPeakValueModifier || data.archetype == Archetype::kDualValueModifier)) {
			if (check_damaging) {
				using Flag = RE::EffectSetting::EffectSettingData::Flag;
				return data.flags.all(Flag::kDetrimental, Flag::kHostile) && data.flags.none(Flag::kRecover);
			} else {
				return true;
			}
		}
		return false;
	}

	bool Hooks::ValidPair(RE::Actor* a_victim, RE::Actor* a_aggressor)
	{
		if (!a_victim->IsHostileToActor(a_aggressor))
			return false;
		if (!Papyrus::Configuration::isnpc(a_victim))
			return false;
		return ValidContender(a_victim) && ValidContender(a_aggressor);
	}

	bool Hooks::ValidContender(RE::Actor* a_actor)
	{
		if (a_actor->IsDead())
			return false;
		return !Configuration::GetSingleton()->isexcludedactor(a_actor);
	}

	void Hooks::ValidateStrip(RE::Actor*, bool)
	{
		// 	const auto settings = Configuration::GetSingleton();
		// 	const auto config = settings->getsettings();
		// 	if (a_gearlist.size() && Kudasai::randomINT<int>(0, 99) < config->stripchance) {
		// 		const auto em = RE::ActorEquipManager::GetSingleton();

		// 		auto item = a_gearlist.at(Kudasai::randomINT<int>(0, static_cast<int>(a_gearlist.size())));
		// 		em->UnequipObject(a_target, item, nullptr, 1, nullptr, true, false, false, true);

		// 		RE::ITEM_REMOVE_REASON reason;
		// 		if (Kudasai::randomINT<int>(0, 99) < config->strpchdstry && settings->isstripprotec(item)) {
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
