#pragma once

namespace Kudasai::Animation
{
	const std::string GetRaceKey(RE::Actor* subject);

	void PlayPaired(const std::vector<RE::Actor*> subjects, const std::vector<std::string> animations);
	void ExitPaired(const std::vector<RE::Actor*> subjects, const std::vector<std::string> animations);

	void PlayAnimation(const RE::Actor* subject, const char* animation);

}  // namespace Kudasai::Animation
