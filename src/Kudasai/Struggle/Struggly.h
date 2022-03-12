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

	// Create a new Struggle Game with the passed in Actors
	// This class is self managing & will delete itself once the Struggle ends
	class Struggly
	{
		Struggly(RE::Actor* victim, RE::Actor* aggressor);//, std::function<void(bool)> callback, std::uint32_t difficulty, StruggleType type);
		~Struggly() = default;

	public:
		/**
		 * @brief Create a new Struggle Interaction between victim & aggressor
		 * 
		 * @param callback A function to call with the result of the Struggle. true means the victim escaped
		 * @param difficulty The difficulty of the game to play; see @type
		 * @param type The type of Struggle. Will be ignored if the Victim is not the Player
		 * 
		 * @throw InvalidCombination If no Struggle Animation can be found
		 */
		static void BeginStruggle(RE::Actor* victim, RE::Actor* aggressor);

	private:
		// const std::function<void(bool)> callback;

		// const StruggleType type;
		// const std::uint32_t difficulty;

		RE::Actor* const victim;
		RE::Actor* const aggressor;
		std::pair<std::string, std::string> animevents;
	};

	class InvalidCombination : public std::exception
	{
		const char* what() const throw()
		{
			return "Actor pair does does not own an Animation.";
		}
	};


}  // namespace Kudasai::Struggle
