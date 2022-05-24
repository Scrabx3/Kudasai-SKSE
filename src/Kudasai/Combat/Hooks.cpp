#include "Kudasai/Combat/Hooks.h"

#include "Kudasai/Combat/Zone.h"
#include "Kudasai/Defeat.h"
#include "Papyrus/Settings.h"

namespace Config = Papyrus::Configuration;
using Archetype = RE::EffectArchetypes::ArchetypeID;

namespace Kudasai
{
	void Hooks::InstallHook()
	{
		SKSE::AllocTrampoline(1 << 7);
		auto& trampoline = SKSE::GetTrampoline();
		// ==================================================
		REL::Relocation<std::uintptr_t> wh{ RELID(37673, 38627) };
		_WeaponHit = trampoline.write_call<5>(wh.address() + OFFSET(0x3C0, 0x4a8), WeaponHit);
		// ================================================== TODO: AE OFFSET
		REL::Relocation<std::uintptr_t> t{ RELID(33763, 42677) };
		_MagicHit = trampoline.write_call<5>(t.address() + OFFSET(0x52F, 0x526), MagicHit);
		// ================================================== TODO: AE OFFSET
		REL::Relocation<std::uintptr_t> ma{ RELID(37832, 37832) };
		_IsMagicImmune = trampoline.write_call<5>(ma.address() + OFFSET(0x3B, 0x3B), IsMagicImmune);
		// ==================================================
		REL::Relocation<std::uintptr_t> det{ RELID(41659, 42742) };
		_DoDetect = trampoline.write_call<5>(det.address() + OFFSET(0x526, 0x67B), DoDetect);
		// ================================================== TODO: AE OFFSET
		REL::Relocation<std::uintptr_t> expl{ RELID(42677, 42677) };
		_ExplosionHit = trampoline.write_call<5>(expl.address() + OFFSET(0x38C, 0x526), ExplosionHit);
		// ==================================================
		REL::Relocation<std::uintptr_t> plu{ RE::PlayerCharacter::VTABLE[0] };
		_PlUpdate = plu.write_vfunc(0xAD, PlUpdate);

		logger::info("Hooks installed");
	}  // InstallHook()

	inline void Hooks::PlUpdate(RE::PlayerCharacter* player, float delta)
	{
		_PlUpdate(player, delta);
		const auto checkdot = [](RE::Actor* victim) -> void {
			const auto& effects = victim->GetActiveEffectList();
			if (!effects)
				return;
			float total = 0.0f;
			for (const auto& effect : *effects) {
				if (!effect || effect->flags.any(RE::ActiveEffect::Flag::kDispelled, RE::ActiveEffect::Flag::kInactive))
					continue;
				else if (const float change = GetExpectedHealthModification(effect); change != 0)
					total += change / 20;  // Only consider damage the spell would do within the next 50ms
			}
			if (total < 0 && victim->GetActorValue(RE::ActorValue::kHealth) <= fabs(total)) {
				if (auto aggressor = GetNearValidAggressor(victim); aggressor) {
					if (GetDefeated(victim, aggressor, true) != HitResult::Proceed) {
						if (Kudasai::Zone::registerdefeat(victim, aggressor)) {
							Defeat::SetDamageImmune(victim);
							RemoveDamagingSpells(victim);
						}
					}
				}
			}
		};

		if (Papyrus::GetSetting<bool>("bEnabled") && Papyrus::Configuration::IsValidPrerequisite()) {
			checkdot(player);

			const auto processLists = RE::ProcessLists::GetSingleton();
			for (auto& handle : processLists->highActorHandles) {
				auto subject = handle.get();
				if (!subject || subject->IsDead() || Defeat::IsDamageImmune(subject.get()) || !Papyrus::Configuration::IsNPC(subject.get()))
					continue;
				checkdot(subject.get());
			}
		}
	}

	void Hooks::WeaponHit(RE::Actor* a_target, RE::HitData& a_hitData)
	{
		const auto aggressor = a_hitData.aggressor.get();
		if (a_target && aggressor && aggressor.get() != a_target && !a_target->IsCommandedActor() && Config::IsNPC(a_target)) {
			logger::info("Weaponhit -> victim = {} ;; aggressor = {}", a_target->GetFormID(), aggressor->GetFormID());
			if (Defeat::IsDamageImmune(a_target)) {
				return;
			} else if (Papyrus::GetSetting<bool>("bEnabled") && Papyrus::Configuration::IsValidPrerequisite()) {
				const float hp = a_target->GetActorValue(RE::ActorValue::kHealth);
				auto dmg = a_hitData.totalDamage + fabs(GetIncomingEffectDamage(a_target));
				AdjustByDifficultyMult(dmg, aggressor->IsPlayerRef());
				const auto t = GetDefeated(a_target, aggressor.get(), hp <= dmg);
				if (t != HitResult::Proceed && Kudasai::Zone::registerdefeat(a_target, aggressor.get())) {
					Defeat::SetDamageImmune(a_target);
					RemoveDamagingSpells(a_target);
					if (t == HitResult::Lethal) {
						if (hp < 2)
							return;
						dmg = hp - 0.05f;
					}
					a_target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -dmg);
					return;
				} else {
					ValidateStrip(a_target, false);
				}
			}
		}
		return _WeaponHit(a_target, a_hitData);
	}  // WeaponHit()

	void Hooks::MagicHit(uint64_t* unk1, RE::ActiveEffect& effect, uint64_t* unk3, uint64_t* unk4, uint64_t* unk5)
	{
		const auto target = effect.GetTargetActor();
		const auto& base = effect.effect ? effect.effect->baseEffect : nullptr;
		if (!target || !base || effect.magnitude >= 0 || !Papyrus::GetSetting<bool>("bEnabled") || !Papyrus::Configuration::IsValidPrerequisite() ||
			target->IsCommandedActor() || !Papyrus::Configuration::IsNPC(target) || !IsDamagingSpell(base->data))
			return _MagicHit(unk1, effect, unk3, unk4, unk5);

		const auto caster = [&]() -> RE::Actor* {
			if (const auto caster = effect.caster.get(); caster)
				return caster.get();
			return nullptr; }();
			// return GetNearValidAggressor(target); }();
		if (caster && caster != target) {
			logger::info("Spellhit -> target = {} ;; caster = {}", target->GetFormID(), caster->GetFormID());
			const float health = target->GetActorValue(RE::ActorValue::kHealth);
			float dmg = base->data.secondaryAV == RE::ActorValue::kHealth ? effect.magnitude * base->data.secondAVWeight : effect.magnitude;
			dmg += GetIncomingEffectDamage(target);	// + GetTaperDamage(effect.magnitude, data->data);
			AdjustByDifficultyMult(dmg, caster->IsPlayerRef());
			const auto type = GetDefeated(target, caster, health <= fabs(dmg));
			if (type != HitResult::Proceed && Kudasai::Zone::registerdefeat(target, caster)) {
				Defeat::SetDamageImmune(target);
				RemoveDamagingSpells(target);
				if (type == HitResult::Lethal) {
					dmg = health - 0.05f;
					effect.magnitude = 0;
					effect.duration = 0;
				}
				target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -dmg);
			} else {
				ValidateStrip(target, true);
			}
		}
		return _MagicHit(unk1, effect, unk3, unk4, unk5);
	}

	// return false if hit should not be processed
	bool Hooks::ExplosionHit(RE::Explosion& explosion, float* flt, RE::Actor* actor)
	{
		if (actor && Defeat::IsDamageImmune(actor))
			return false;

		return _ExplosionHit(explosion, flt, actor);
	}

	bool Hooks::IsMagicImmune(RE::Actor* target, RE::MagicItem* item)
	{
		if (!target || !item || target->IsDead())
			return _IsMagicImmune(target, item);

		enum {
			damaging,
			healing,
			none
		};

		for (auto& effect : item->effects) {
			auto base = effect ? effect->baseEffect : nullptr;
			if (!base)
				continue;
			auto& data = base->data;
			const auto isdamaging = [&]() -> int {
				if (data.primaryAV == RE::ActorValue::kHealth || data.secondaryAV == RE::ActorValue::kHealth)
					return (data.flags.underlying() & 4) == 4 ? damaging : healing;	// 4 = kDetrimental
				return none;
			};

			if (data.archetype != RE::EffectSetting::Archetype::kScript && Defeat::IsDamageImmune(target)) {
				if (isdamaging() == healing)  // Some positive effect on Hp
					Defeat::rescue(target, true);
				else
					return true;
			}
		}
		return _IsMagicImmune(target, item);
	}

	uint8_t* Hooks::DoDetect(RE::Actor* viewer, RE::Actor* target, int32_t& detectval, uint8_t& unk04, uint8_t& unk05, uint32_t& unk06, RE::NiPoint3& pos, float& unk08, float& unk09, float& unk10)
	{
		if (viewer && target && (Defeat::ispacified(viewer) || Defeat::ispacified(target))) {
			detectval = -1000;
			return nullptr;
		}
		return _DoDetect(viewer, target, detectval, unk04, unk05, unk06, pos, unk08, unk09, unk10);
	}

	Hooks::HitResult Hooks::GetDefeated(RE::Actor* a_victim, RE::Actor* a_aggressor, const bool lethal)
	{
		if (!ValidPair(a_victim, a_aggressor))
			return HitResult::Proceed;

		static const auto hunterpride = RE::TESDataHandler::GetSingleton()->LookupForm<RE::EffectSetting>(0x933DA3, ESPNAME);
		if (a_aggressor->IsPlayerRef() && (!a_aggressor->HasMagicEffect(hunterpride) || !lethal))
			return HitResult::Proceed;

		if (lethal) {
			using Flag = RE::Actor::BOOL_FLAGS;
			bool protecc;
			if (Papyrus::GetSetting<bool>("bLethalEssential") && (a_victim->boolFlags.all(Flag::kEssential) || (!a_aggressor->IsPlayerRef() && a_victim->boolFlags.all(Flag::kProtected))))
				protecc = true;
			else if (a_victim->IsPlayerRef())
				protecc = Random::draw<float>(0, 99.5f) < Papyrus::GetSetting<float>("fLethalPlayer");
			else
				protecc = Random::draw<float>(0, 99.5f) < Papyrus::GetSetting<float>("fLethalNPC");
			logger::info("Incomming Hit is lethal; Protecting ? = {}", protecc);
			return protecc ? HitResult::Lethal : HitResult::Proceed;
		} else {
			// TODO: rework exposed algorithm
			// if (/* exposed? */ wornarmor.size() < config->armorthresh ||
			// 	/* exhausted stamina? */ Kudasai::getavpercent(a_victim, RE::ActorValue::kStamina) < config->staminathresh ||
			// 	/* exhausted magicka? */ Kudasai::getavpercent(a_victim, RE::ActorValue::kMagicka) < config->magickathresh)
			// 	return HitResult::Defeat;
		}
		logger::info("No Valid Defeat Conditions for Victim = {}", a_victim->GetFormID());
		return HitResult::Proceed;
	}

	const float Hooks::GetTaperDamage(const float magnitude, const RE::EffectSetting::EffectSettingData& data)
	{
		return magnitude * data.taperWeight * data.taperDuration / (data.taperCurve + 1);
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
			else if (const float change = GetExpectedHealthModification(effect); change != 0.0f) {
				ret += change;
			}
		}
		return ret;
	}

	const float Hooks::GetExpectedHealthModification(RE::ActiveEffect* a_effect)
	{
		const auto base = a_effect->GetBaseObject();
		if (!base || base->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kRecover))
			return 0.0f;
		// Damge done every second by the effect
		const auto getmagnitude = [&]() -> float {
			if (a_effect->duration - base->data.taperDuration < a_effect->elapsedSeconds)
				return GetTaperDamage(a_effect->magnitude, base->data);
			else
				return a_effect->magnitude;
		};
		if (base->data.primaryAV == RE::ActorValue::kHealth)
			return getmagnitude();
		else if (base->data.secondaryAV == RE::ActorValue::kHealth)	 // only DualValueModifier can have an ActorValue as 2nd Item
			return getmagnitude() * base->data.secondAVWeight;
		else
			return 0.0f;
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
			if (base && IsDamagingSpell(base->data)) {
				logger::info("Dispelling Spell = {}", base->GetFormID());
				eff->Dispel(true);
			}
		}
	}

	bool Hooks::IsDamagingSpell(const RE::EffectSetting::EffectSettingData& data)
	{
		if (data.primaryAV == RE::ActorValue::kHealth || data.secondaryAV == RE::ActorValue::kHealth)
			return (data.flags.underlying() & 6) == 4;
		return false;
	}

	bool Hooks::ValidPair(RE::Actor* a_victim, RE::Actor* a_aggressor)
	{
		if (!a_victim->IsHostileToActor(a_aggressor))
			return false;
		return ValidContender(a_victim) && ValidContender(a_aggressor);
	}

	bool Hooks::ValidContender(RE::Actor* a_actor)
	{
		if (a_actor->IsDead())
			return false;
		return Config::IsValidActor(a_actor);
	}

	void Hooks::AdjustByDifficultyMult(float& damage, const bool playerPOV)
	{
		const auto s = RE::GetINISetting("iDifficulty:GamePlay");
		if (s->GetType() != RE::Setting::Type::kSignedInteger)
			return;

		std::string diff{ "fDiffMultHP"s + (playerPOV ? "ByPC"s : "ToPC"s) };
		switch (s->GetSInt()) {
		case 0:
			diff.append("VE");
			break;
		case 1:
			diff.append("E");
			break;
		case 2:
			diff.append("N");
			break;
		case 3:
			diff.append("H");
			break;
		case 4:
			diff.append("VH");
			break;
		case 5:
			diff.append("L");
			break;
		default:
			logger::error("Invalid Difficulty Setting");
			return;
		}
		const auto smult = RE::GameSettingCollection::GetSingleton()->GetSetting(diff.c_str());
		if (smult->GetType() != RE::Setting::Type::kFloat)
			return;

		const auto mult = smult->GetFloat();
		damage *= mult;
	}

	RE::Actor* Hooks::GetNearValidAggressor(RE::Actor* victim)
	{
		const auto& tarP = victim->GetPosition();
		const auto valid = [&](RE::Actor* subject) {
			return subject->IsHostileToActor(victim) && subject->IsInCombat() && subject->GetPosition().GetDistance(tarP) < 4096.0f;
		};
		const auto& processLists = RE::ProcessLists::GetSingleton();
		for (auto& pHandle : processLists->highActorHandles) {
			auto potential = pHandle.get().get();
			if (!potential || potential->IsDead() || Defeat::IsDamageImmune(potential) || !valid(potential))
				continue;
			if (const auto group = potential->GetCombatGroup(); group) {
				for (auto& e : group->targets)
					if (e.targetHandle.get().get() == victim)
						return potential;
			}
		}
		const auto player = RE::PlayerCharacter::GetSingleton();
		return valid(player) ? player : nullptr;
	}

	void Hooks::ValidateStrip(RE::Actor*, bool)
	{
		// 	const auto settings = Configuration::GetSingleton();
		// 	const auto config = settings->getsettings();
		// 	if (a_gearlist.size() && Kudasai::Random::draw<int>(0, 99) < config->stripchance) {
		// 		const auto em = RE::ActorEquipManager::GetSingleton();

		// 		auto item = a_gearlist.at(Kudasai::Random::draw<int>(0, static_cast<int>(a_gearlist.size())));
		// 		em->UnequipObject(a_target, item, nullptr, 1, nullptr, true, false, false, true);

		// 		RE::ITEM_REMOVE_REASON reason;
		// 		if (Kudasai::Random::draw<int>(0, 99) < config->strpchdstry && settings->isstripprotec(item)) {
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
