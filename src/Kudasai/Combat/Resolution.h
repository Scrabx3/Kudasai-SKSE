#pragma once

namespace Kudasai
{
	class Resolution
	{
		struct ResData
		{
			RE::TESQuest* quest;
		};

	public:
		static Resolution* GetSingleton()
		{
			static Resolution singleton;
			return &singleton;
		}

		static void Register();

	public:
		std::vector<ResData> friendly;
		std::vector<ResData> hostile;

	private:
		Resolution() = default;
		~Resolution() = default;

		bool ReadFileData(const std::string& filepath, std::vector<ResData>& list);
	};

}  // namespace Kudasai
