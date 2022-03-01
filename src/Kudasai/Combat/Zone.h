#pragma once

#include "Kudasai/Defeat.h"

namespace Kudasai
{
	class Zone
	{
		enum class DefeatResult
		{
			Cancel,
			Resolution,
			Defeat,
			Assault
		};

	public:
		[[nodiscard]] static Zone* GetSingleton()
		{
			static Zone singleton;
			return &singleton;
		}
		
		/**
		 * @brief this actor to be defeated, update its Zone & decide type of defeat
		 * 
		 * @return if the defeat was successfully registered. Reason for failure are zone not being able to update or no valid defeat pattern
		 */
		static bool registerdefeat(RE::Actor* victim, RE::Actor* aggressor);

	private:
		void defeat(RE::Actor* victim, RE::Actor* aggressor, DefeatResult result);

		DefeatResult getdefeattype(RE::CombatGroup* agrzone);
		int countvalid(RE::BSTArray<RE::CombatGroup::TargetData>& list);
		int countvalid(RE::BSTArray<RE::CombatGroup::MemberData>& list);
		bool valid(RE::ActorPtr& that)
		{
			return that && !that->IsCommandedActor() && !Defeat::isdefeated(that.get());
		}
	};	// class ZoneFactory

}