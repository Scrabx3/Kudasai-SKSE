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

		/** 
		 * @param callback void(bool, this): Invoked on struggle end. bool = true when Victim won
		 * @param difficulty The difficulty of the game to play. 
		 * @param type The type of Struggle. Will be ignored if the Victim is not the Player
		 * 
		 * @throw InvalidCombination If the victim is not a NPC or no Animationcan is found
		 */
		static Struggle* CreateStruggle(CallbackFunc callback, std::vector<RE::Actor*> actors, int difficulty, StruggleType type);
		static void DeleteStruggle(Struggle* struggle);

		/**
		 * @brief Play a breakfree Animation. This will end the Struggle, set the Actors free again. The instance will commit suicide afterwards
		 * 
		 * @param anims The animations with which to end the Struggle. { animations.size() == actors.size() } must hold 
		 */
		static void PlayBreakfree(std::vector<RE::Actor*> positions) noexcept;
		static void PlayBreakfree(std::vector<RE::Actor*> positions, std::vector<std::string> anims) noexcept;

		_NODISCARD static Struggle* FindPair(RE::Actor* subject);
		static inline std::vector<std::unique_ptr<Struggle>> strugglers;

	public:
		Struggle(CallbackFunc callback, std::vector<RE::Actor*> actors, int difficulty, StruggleType type);
		~Struggle() noexcept;

		/**
		 * @brief Forcefully end the Struggle early, invoking its Callback
		 * 
		 * @param defeated The Actor which will lose the Struggle
		 */
		void StopStruggle(RE::Actor* defeated) noexcept;

	public:
		std::vector<RE::Actor*> actors;

	private:
		const CallbackFunc callback;

		bool active;
		std::thread _t;
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
