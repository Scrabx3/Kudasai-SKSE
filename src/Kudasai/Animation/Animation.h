#pragma once

namespace Kudasai::Animation
{
	const std::string GetRaceKey(RE::Actor* subject);

	void PlayPaired(RE::Actor* first, RE::Actor* partner, const std::pair<std::string, std::string> animations);
	void ExitPaired(RE::Actor* first, RE::Actor* partner, const std::pair<std::string, std::string> animations);

	void PlayAnimation(RE::Actor* subject, const char* animation);

}  // namespace Kudasai::Animation
