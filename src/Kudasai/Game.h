#pragma once

#include "Kudasai/Combat/Zone.h"

namespace Kudasai
{
	namespace Games
	{
		// TODO: what happens when a struggle is lost? how is it communicated?

		static bool Struggle(RE::Actor* victim, RE::Actor* aggressor, int difficulty = 0);

	}  // namespace Games

}  // namespace Kudasai
