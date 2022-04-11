#pragma once

// #include "Kudasai/Interface/QTE.h"
#include "Kudasai/Animation/Animation.h"
#include "Papyrus/Settings.h"

namespace Kudasai
{
	class Struggle
	{
		using CallbackFunc = std::function<void(bool, Struggle*)>;

	public:
		enum class StruggleType
		{
			None,
			QTE,
			Input
		};

		// enum class StruggleResult
		// {
		// 	None = 1 << 0,		 // No Result yet
		// 	ForceNext = 1 << 1,	 // player struggles only; the next callback will end the struggle independend of game state
		// 	Victoire = 1 << 30,	 // Victoire won
		// 	Victim = 1 << 31	 // Victim won
		// };

		/**
		 * @brief Find the counterpart animating with this Actor
		 * 
		 * @return The class object containing the subject, or nullptr if no object holds this actor
		 */
		_NODISCARD static Struggle* FindPair(RE::Actor* subject);
		static inline std::vector<Struggle*> strugglers;

	public:
		/**
		 * @param callback A function to call once this struggle is completed, holding result (true => victim won) & this
		 * 
		 * @throw InvalidCombination If the victim is not a NPC or no Animationcan be found for playing
		 * 
		 */
		Struggle(CallbackFunc callback, RE::Actor* victim, RE::Actor* aggressor);
		~Struggle() noexcept;

		/**
		 * @brief Begin the struggle game & play the animation
		 * 
		 * @param difficulty The difficulty of the game to play. NONE: Chance to escape QTE: Time to react INPUT: ???
		 * @param type The type of Struggle. Will be ignored if the Victim is not the Player
		 * 
		 */
		void BeginStruggle(double difficulty = 2.3, StruggleType type = StruggleType::None);

		/**
		 * @brief Forcefully stop the current struggle & invoke the Callback func
		 * 
		 * @param defeated Must be either nullptr, @victim or @aggressor. Represents the actor which lost the struggle
		 * 
		 */
		void StopStruggle(RE::Actor* defeated) noexcept;

		/** COMEBACK: necessary? Not sure if that should be a thing with Struggles being essentially removed for the time being
		 * if implementing, will also need to implement StruggleResult interactions which currently arent respected either
		 * 
		 * @brief Force the result of the Struggle, without stopping it.
		 * 
		 * @param result See @StruggleResult, Passing 'StruggleResult::None' is UB
		 * 
		 */
		// void PredefineResult(StruggleResult define) noexcept;

		/**
		 * @brief Play the breakfree animation. To use instead of a PlayAnimation()
		 * 
		 */
		void PlayBreakfree() noexcept;

	public:
		RE::Actor* const victim;
		RE::Actor* const aggressor;

		// StruggleResult result;

	private:
		bool active;
		std::thread _t;
		std::pair<std::string, std::string> animations;
		CallbackFunc callback;
	};

	class InvalidCombination : public std::exception
	{
		const char* what() const throw()
		{
			ConsolePrint("[Kudasai] Could not start Struggle Animation; no animation found.");
			return "Actor pair does does not own an Animation.";
		}
	};

}  // namespace Kudasai::Struggle
