#pragma once

namespace Kudasai
{
	class Hooks
	{
	public:
		static void InstallHook();

	private:
		enum class HitResult
		{
			Proceed,  // No Defeat, Proceed
			Lethal,	  // Defeat, Lethal hit
			Defeat	  // Defeat, no lethal hit
		};

		static void PlUpdate(RE::PlayerCharacter* player, float delta);
		static void UpdateCombat(RE::Character* a_this, RE::Actor* unk01, RE::Actor* unk02, RE::Actor* unk03);
		static void WeaponHit(RE::Actor* a_target, RE::HitData& a_hitData);
		static void MagicHit(uint64_t* unk1, RE::ActiveEffect& effect, uint64_t* unk3, uint64_t* unk4, uint64_t* unk5);
		static bool DoesMagicHitApply(RE::MagicTarget* a_target, RE::MagicTarget::AddTargetData* a_data);
		static bool ExplosionHit(RE::Explosion& explosion, float* flt, RE::Actor* actor);
		static uint8_t* DoDetect(RE::Actor* viewer, RE::Actor* target, int32_t& detectval, uint8_t& unk04, uint8_t& unk05, uint32_t& unk06, RE::NiPoint3& pos, float& unk08, float& unk09, float& unk10);

		static void CalcDamageOverTime(RE::Actor* a_target); 
		static HitResult GetDefeated(RE::Actor* a_victim, RE::Actor* a_aggressor, const bool lethal);
		static const float GetTaperDamage(const float magnitude, const RE::EffectSetting::EffectSettingData& data);
		static const float GetIncomingEffectDamage(RE::Actor* subject);
		static const float GetExpectedHealthModification(RE::ActiveEffect* a_effect);
		static void RemoveDamagingSpells(RE::Actor* subject);
		static bool IsDamagingSpell(const RE::EffectSetting::EffectSettingData& data);

		static bool ValidPair(RE::Actor* a_victim, RE::Actor* a_aggressor);
		static bool ValidContender(RE::Actor* a_actor);

		static void ValidateStrip(RE::Actor* target);

		static void AdjustByDifficultyMult(float& damage, const bool playerPOV);
		static RE::Actor* GetNearValidAggressor(RE::Actor* a_victim);

		static inline REL::Relocation<decltype(PlUpdate)> _PlUpdate;
		static inline REL::Relocation<decltype(UpdateCombat)> _UpdateCombat;
		static inline REL::Relocation<decltype(WeaponHit)> _WeaponHit;
		static inline REL::Relocation<decltype(MagicHit)> _MagicHit;
		static inline REL::Relocation<decltype(DoesMagicHitApply)> _DoesMagicHitApply;
		static inline REL::Relocation<decltype(ExplosionHit)> _ExplosionHit;
		static inline REL::Relocation<decltype(DoDetect)> _DoDetect;
	};

}  // namespace Hooks
