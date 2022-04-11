namespace Kudasai::Resolution
{
	class QuestData
	{
	public:
		QuestData(const std::string filepath);
		~QuestData() = default;

		_NODISCARD const std::string GetName() const noexcept;
		_NODISCARD const int32_t GetWeight();

		_NODISCARD const bool CanBlackout() const;
		_NODISCARD const bool MatchesRace(std::vector<RE::Actor*> list) const;

		void UpdateWeight(const int32_t value);
		void WriteFile();

		RE::TESQuest* quest;  // is null if requirements arent met

	private:
		const std::string filepath;
		YAML::Node root;
	};

	class InvalidConfig : public std::exception
	{
	public:
		InvalidConfig(const char* error) :
			error(error) {}
		~InvalidConfig() = default;

		const char* error;

		virtual const char* what() const throw()
		{
			ConsolePrint(error);
			return error;
		}
	};

	void Register();
	void UpdateProperties();
	void UpdateWeights(const std::vector<int32_t>& list);

	// Wrapper functions for SelectQuest()
	RE::TESQuest* GetQuestHostile(std::vector<RE::Actor*> list, bool blackout);
	RE::TESQuest* GetQuestFriendly(std::vector<RE::Actor*> list);

	RE::TESQuest* SelectQuest(std::vector<QuestData>& quests, std::vector<RE::Actor*>& list, bool blackout);

	static inline std::vector<QuestData> FriendlyQuests;
	static inline std::vector<QuestData> HostileQuests;
}  // namespace Kudasai::Resolution