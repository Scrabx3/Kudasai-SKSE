#pragma once

namespace Kudasai::Animation
{
	const std::string GetRaceKey(RE::Actor* subject);

	RE::TESObjectREFR* const GetRootObject(RE::TESObjectREFR* location);
	void PlayPaired(RE::Actor* first, RE::Actor* partner, const std::pair<std::string, std::string> animations);
	void ExitPaired(RE::Actor* first, RE::Actor* partner, const std::pair<std::string, std::string> animations);

	void PlayAnimation(RE::Actor* subject, const char* animation);

	class ObjectCreationFailure : public std::exception
	{
		const char* what() const throw()
		{
			return "Could not create object for class.";
		}
	};

}  // namespace Kudasai::Animation
