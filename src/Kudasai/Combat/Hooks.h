#pragma once

namespace Kudasai
{
	class Hooks
	{
	public:
		static void InstallHook();

	private:
		enum HitResult
		{
			Proceed,  // No Defeat, Proceed
			Lethal,	  // Defeat, Lethal hit
			Defeat	  // Defeat, no lethal hit
		};

		static void WeaponHit(RE::Actor* a_target, RE::HitData& a_hitData);
		static char MagicHit(RE::MagicTarget* a_target, RE::MagicTarget::CreationData* a_data);
		static bool IsMagicImmune(RE::Actor* target, RE::MagicItem* item);
		static uint8_t* DoDetect(RE::Actor* viewer, RE::Actor* target, int32_t& detectval, uint8_t& unk04, uint8_t& unk05, uint32_t& unk06, RE::NiPoint3& pos, float& unk08, float& unk09, float& unk10);

		static HitResult GetDefeated(RE::Actor* a_victim, RE::Actor* a_aggressor, const bool lethal);
		static const float GetTaperDamage(const float magnitude, const RE::EffectSetting::EffectSettingData& data);
		static const float GetIncomingEffectDamage(RE::Actor* subject);
		static void RemoveDamagingSpells(RE::Actor* subject);
		static bool SpellModifiesHealth(RE::EffectSetting::EffectSettingData& data, const bool check_damaging);

		static bool ValidPair(RE::Actor* a_victim, RE::Actor* a_aggressor);
		static bool ValidContender(RE::Actor* a_actor);

		static void ValidateStrip(RE::Actor* target, bool magic);

		static void AdjustByDifficultyMult(float& damage, const bool playerPOV);

		static inline REL::Relocation<decltype(WeaponHit)> _WeaponHit;
		static inline REL::Relocation<decltype(MagicHit)> _MagicHit;
		static inline REL::Relocation<decltype(IsMagicImmune)> _IsMagicImmune;
		static inline REL::Relocation<decltype(DoDetect)> _DoDetect;
	};
}  // namespace Hooks
