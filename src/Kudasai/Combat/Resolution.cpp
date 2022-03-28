#include "Kudasai/Combat/Resolution.h"

namespace Kudasai
{
	void Resolution::Register()
	{
		auto me = GetSingleton();
		// Cycle through all Files in Friendly/Hostile Post Combat Folders & create their own Res Data
		// then update the Array in Papyrus listing all of the outcomes
		// 1 -> Friendly
		const auto friendlies = fs::path{ CONFIGPATH("PostCombat\\Friendly") };
		for (auto& file : fs::directory_iterator{ friendlies }) {
			const auto path = file.path().string();
			logger::info("Reading Friendly File = {}", path);
			if(!me->ReadFileData(path, me->friendly))
        logger::error("Could not create Resolution Data from file.");
		}
    // 2 -> Hostile
		const auto hostiles = fs::path{ CONFIGPATH("PostCombat\\Hostile") };
		for (auto& file : fs::directory_iterator{ hostiles }) {
			const auto path = file.path().string();
			logger::info("Reading Hostile File = {}", path);
			if (!me->ReadFileData(path, me->hostile))
				logger::error("Could not create Resolution Data from file.");
		}
	}

	bool Resolution::ReadFileData(const std::string&, std::vector<ResData>&)
	{
		return false;
  }
}  // namespace Kudasai
