#pragma once

// #include "Kudasai/Interface/QTE.h"
#include "Kudasai/Animation/Animation.h"
#include "Papyrus/Settings.h"

namespace Kudasai::Struggle
{
	enum class StruggleType
	{
		None,
		QTE,
		Input
	};

	/**
	 * @brief Create a new Struggle Interaction between victim & aggressor
	 * 
	 * @param callback A function to call with the result of the Struggle. true means the victim escaped
	 * @param difficulty The difficulty of the game to play. NONE: Chance to escape QTE: Time to react INPUT: ???
	 * @param type The type of Struggle. Will be ignored if the Victim is not the Player
	 * 
	 */
	void BeginStruggle(std::function<void(bool)> callback, double difficulty = 2.3, StruggleType type = StruggleType::None);

	/**
	 * @brief Play a Struggle Animation or break free from one. This will restrain the actor, or set the player AI Driven
	 * 
	 * @throw InvalidCombination "PlayStruggle" only -> When no Animation can be found
	 */
	void PlayStruggle(RE::Actor* victim, RE::Actor* aggressor);
	void PlayBreakfree(RE::Actor* victim, RE::Actor* aggressor) noexcept;

	class InvalidCombination : public std::exception
	{
		const char* what() const throw()
		{
			ConsolePrint("[Kudasai] Could not start Struggle Animation; no animation found.");
			return "Actor pair does does not own an Animation.";
		}
	};


}  // namespace Kudasai::Struggle
