#pragma once

namespace Kudasai::Animation
{
	/**
	 * @brief Create and return the RaceKey for this specific Actor. See RaceKey.yaml
	 * 
	 */
	const std::string GetRaceKey(RE::Actor* subject);

	/**
	 * @brief Lookup valid Struggle Animations for the given positions
	 * 
	 * @param positions [0] must be a NPC, [1]+ are expected to use the same RaceKey
	 * @throw InvalidAnimationRequest If no Animation for this Combination can be found
	 * 
	 */
	std::vector<std::string> LookupStruggleAnimations(std::vector<RE::Actor*> positions);
	std::vector<std::string> LookupBreakfreeAnimations(std::vector<RE::Actor*> positions) noexcept;
	std::vector<std::string> LookupKnockoutAnimations(std::vector<RE::Actor*> positions) noexcept;

	/**
	 * @brief Set or release a group of Actors to play for pseudo paired animations
	 * 
	 */
	void SetPositions(const std::vector<RE::Actor*> positions);
	void ClearPositions(const std::vector<RE::Actor*> positions);

	/**
	 * @brief Wrapper for thread save NotifyAnimationGraph() calls
	 * 
	 */
	void PlayAnimation(const RE::Actor* subject, const char* animation);

	class InvalidAnimationRequest : public std::exception
	{
		const char* what() const throw()
		{
			ConsolePrint("[Kudasai] Invalid Animation Prerequisite.");
			return "Actor Group does does not own an Animation.";
		}
	};
}  // namespace Kudasai::Animation
