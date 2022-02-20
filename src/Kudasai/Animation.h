#pragma once

namespace Kudasai
{
	class Animation
	{
	public:
		/**
		 * @brief play a struggle animation between two actors
		 * 
		 * @param victim The actor being attacked in the animation
		 * @param aggressor The actor doing the attacking
		 * @return if an animation started. E.g. no matching animation is found
		 */
    static bool PlayStruggle(RE::Actor* victim, RE::Actor* aggressor);

	private:
		Animation() = default;
		~Animation() = default;
	};
} // namespace Kudasai
