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
		/**
		 * @brief this actor to be defeated, update its Zone & decide type of defeat
		 * 
		 * @return if the defeat was successfully registered. Reason for failure are zone not being able to update or no valid defeat pattern
		 */
		static bool registerdefeat(RE::Actor* victim, RE::Actor* aggressor);

	private:
		static void defeat(RE::Actor* victim, RE::Actor* aggressor, DefeatResult result);
		static DefeatResult getdefeattype(RE::CombatGroup* agrzone);

		template <typename T>  // Element at &T+0 must be an ActorHandla
		static int countvalid(RE::BSTArray<T>& list)
		{
			int ret = 0;
			for (auto& target : list) {
				auto ptr = ((RE::ActorHandle*)(&target))->get();
				if (ptr)
					logger::info("ActorHandle found? Form ID = {}", ptr->GetFormID());
				if (ptr && !ptr->IsCommandedActor() && !Defeat::isdefeated(ptr.get()))
					ret++;
			}
			return ret;
		}
	};	// class ZoneFactory

}