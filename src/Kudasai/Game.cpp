
#include "Kudasai/Game.h"

namespace Kudasai
{
	namespace Games
	{
		template <class reward, class lose>
		bool Struggle(RE::Actor* victim, RE::Actor* aggressor, int difficulty = 0)
    {
      // a Struggle Game starts by first checking if the animation can start, then by diving player & non player for the actual UI minigame
      // if the victim is not the player simply create a % chance that the victim escapes after some time, otherwise let Flash do its magic
      // when the actor loses, they should be "defeated" <- what exactly does this mean?
      // otherwise they recover and re enter combat <- how exactly does this work with Zones?
      return reward(victim, aggressor);
    }
	}  // namespace Games
  
}  // namespace Kudasai