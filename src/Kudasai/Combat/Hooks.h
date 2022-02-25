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
		static uint8_t MagicHit(RE::MagicTarget* a_target, RE::MagicTarget::CreationData* a_data);
		static bool IsGhostMagic(RE::Actor* target, RE::MagicItem* item);
		static uint8_t* DoDetect(RE::Actor* viewer, RE::Actor* target, int32_t& detectval, uint8_t& unk04, uint8_t& unk05, uint32_t& unk06, RE::NiPoint3& pos, float& unk08, float& unk09, float& unk10);

		static HitResult getDefeated(RE::Actor* a_victim, RE::Actor* a_aggressor, std::vector<RE::TESObjectARMO*> wornarmor, const bool lethal);
		static bool spellmodifieshp(RE::EffectSetting::EffectSettingData& data);

		static bool ValidContender(RE::Actor* a_actor);
		static bool ValidPair(RE::Actor* a_victim, RE::Actor* a_aggressor);

		static void validatestrip(RE::Actor* target, std::vector<RE::TESObjectARMO*> worn, bool magic);
		static void removedamagingspells(RE::Actor* subject);

		static inline REL::Relocation<decltype(WeaponHit)> _WeaponHit;
		static inline REL::Relocation<decltype(MagicHit)> _MagicHit;
		static inline REL::Relocation<decltype(IsGhostMagic)> _IsGhostMagic;
		static inline REL::Relocation<decltype(DoDetect)> _DoDetect;
	};
}  // namespace Hooks
