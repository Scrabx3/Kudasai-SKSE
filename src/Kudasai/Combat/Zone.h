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
		bool registerdefeat(RE::Actor* victim, RE::Actor* aggressor);

		// /**
		//  * @brief Create & store a Zone object
		//  * 
		//  * @return the newly created Zone
		//  * @throw null_zone - if the Zone can't be populated
		//  */
		// Zone& createzone(RE::Actor* subject);

		// /**
		//  * @brief Find this Actor inside a Zone
		//  * 
		//  * @return the Zone this Actor belongs too, null if this Actor doesnt belong to any Zone 
		//  */
		// Zone* findzone(RE::Actor* subject);

	private:
		void resolution(RE::Actor* victim, RE::Actor* aggressor);
		void assault(RE::Actor* victim, RE::Actor* aggressor);
		void defeat(RE::Actor* victim);

		DefeatResult getdefeattype(RE::CombatGroup* agrzone);
		int countvalid(RE::BSTArray<RE::CombatGroup::TargetData> list);
		int countvalid(RE::BSTArray<RE::CombatGroup::MemberData> list);
		bool valid(RE::ActorPtr& that)
		{
			return that && !that->IsCommandedActor() && !Defeat::isdefeated(that.get());
		}

		// std::vector<Zone> zones; // TODO: delete
		std::mutex _m;
	};	// class ZoneFactory
}

// 	class Zone
// 	{
// 	public:
// 		struct MemberData
// 		{
// 			enum class Status
// 			{
// 				Active,
// 				Defeated,
// 				Processing,	// currently handled
// 				Grace,			// returned to combat but imune to knockdown
// 				Commanded		// summons shouldnt affect zone evaluation as they are directly dependend on their summoner
// 			};
// 			MemberData(RE::Actor* subject)
// 			{
// 				if (subject->IsCommandedActor())
// 					status = Status::Commanded;
// 				else
// 					status = Status::Active;

// 				// TODO: define Valid Race
// 			}
// 			~MemberData() = default;

// 			bool ValidRace;
// 			Status status;
// 		};

// 		enum class ZoneStatus
// 		{
// 			Cancel,		 // no assault permitted
// 			Resolution,	 // combat is over, the last knockdown defeated the final opposing member of this zone
// 			Assault		 // combat is no over but an assault is permitted
// 		};

// 		// formid -> Actor + MemberData
// 		using Alliance = std::unordered_map<int, std::pair<RE::Actor*, Zone::MemberData>>;

// 	public:
// 		Zone(RE::Actor* subject);
// 		void ZoneUpdate();
// 		~Zone() = default;

// 		/**
// 	 * @brief Checks if the given Actor is part of this Zone (belongs to an Alliance)
// 	 * 
// 	 * @return The Alliance the Actor belongs to (nullptr if they aren't part of this Zone)
// 	 */
// 		Alliance* allianceof(RE::Actor*);

// 		/**
// 		 * @brief checks alliance relations to see if combat ended and/or an aggressor may assault their victim
// 		 * 
// 		 * @return if an assault is permitted
// 		 */
// 		ZoneStatus allowassault(RE::Actor* aggressor);

// 		/**
// 		 * @brief create (mid combat) assault & ultimately defeat this victim
// 		 * 
// 		 */
// 		void assault(RE::Actor* victim, RE::Actor* aggressor);

// 		/**
// 		 * @brief create (post combat) assault
// 		 * 
// 		 */
// 		void resolution(RE::Actor* victim, RE::Actor* aggressor);

// 		/**
// 		 * @brief get the memberdata for this actor
// 		 * 
// 		 * @return the associated member data, or null if the actor isnt part of this Zone
// 		 */
// 		MemberData* getmemberdata(RE::Actor* subject);

// 		/**
// 		 * @brief flag this actor as defeated & lock them into bleedout. Create standup conditions
// 		 * 
// 		 */
// 		void defeatactor(RE::Actor* subject);

// 		/**
// 		 * @brief return this actor back into combat, restoring 20% of their maximum HP
// 		 * 
// 		 */
// 		void restoreactor(RE::Actor* subject);

// 	private:
// 		/**
// 		 * @brief put this actor back into combat after a short delay, healing them by 20% of their max hp
// 		 * 
// 		 * @param subject the actor to put recover
// 		 * @param data memberdata whichs to set
// 		 */
// 		static void graceperiod(RE::Actor* subject, MemberData* data);

// 		std::vector<Alliance> alliances;
// 	};	// class Zone

// }  // namespace Kudasai

// class null_zone : public std::exception
// {
// public:
// 	const char* what() const noexcept override
// 	{
// 		return "Zone population failed, no Combat Group on passed subject?\n";
// 	}
// };	// exception class null_zone
