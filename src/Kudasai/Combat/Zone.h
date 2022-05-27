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
		static DefeatResult getdefeattype(RE::Actor* aggressor);
		static bool CreatePlayerResolution(RE::Actor* aggressor, bool blackout);
		static void CreateNPCResolution(RE::Actor* aggressor);

		class PlayerDefeat :
			public Singleton<PlayerDefeat>
		{
		public:
			static void Register();
			static void Unregister();
			bool Active;
		private:
			void Cycle();
		};	// class PlayerDefeat

		inline static std::mutex _m;
	};	// class Zone

	// Anti Softlock mechanism, call when the Player is defeated (non post combat) to avoid issues in which combat may end without
	// a clear result (e.g. death by falldamage, traps or running away), causing the player to become soft locked in Bleedout
	
}