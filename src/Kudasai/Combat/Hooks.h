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
		static bool ExplosionHit(RE::Explosion& explosion, float* flt, RE::Actor* actor);

		static HitResult GetDefeated(RE::Actor* a_victim, RE::Actor* a_aggressor, const bool lethal);
		static const float GetTaperDamage(const float magnitude, const RE::EffectSetting::EffectSettingData& data);
		static const float GetIncomingEffectDamage(RE::Actor* subject);
		static void RemoveDamagingSpells(RE::Actor* subject);
		static bool SpellModifiesHealth(const RE::EffectSetting::EffectSettingData& data, const bool check_damaging);

		static bool ValidPair(RE::Actor* a_victim, RE::Actor* a_aggressor);
		static bool ValidContender(RE::Actor* a_actor);

		static void ValidateStrip(RE::Actor* target, bool magic);

		static void AdjustByDifficultyMult(float& damage, const bool playerPOV);
		static RE::Actor* GetNearValidAggressor(RE::Actor* victim);

		static inline REL::Relocation<decltype(WeaponHit)> _WeaponHit;
		static inline REL::Relocation<decltype(MagicHit)> _MagicHit;
		static inline REL::Relocation<decltype(IsMagicImmune)> _IsMagicImmune;
		static inline REL::Relocation<decltype(ExplosionHit)> _ExplosionHit;
		static inline REL::Relocation<decltype(DoDetect)> _DoDetect;


		static void Test(uint64_t* unk1, RE::ActiveEffect& effect, uint64_t* unk3, uint64_t* unk4, uint64_t* unk5);
		static inline REL::Relocation<decltype(Test)> _Test;

		static char AVTest(RE::ActiveEffect* effect);
		static inline REL::Relocation<decltype(AVTest)> _AVTest;

		static void PlUpdate(RE::PlayerCharacter* player, float delta);
		static inline REL::Relocation<decltype(PlUpdate)> _PlUpdate;
	};

	inline char Hooks::AVTest(RE::ActiveEffect* effect)
	{
		return _AVTest(effect);
	}

}  // namespace Hooks
