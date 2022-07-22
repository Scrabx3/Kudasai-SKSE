namespace Kudasai
{

	class Resolution
	{
		struct QuestData
		{
		public:
			QuestData(const std::string filepath);
			~QuestData() = default;

			_NODISCARD const std::string GetName() const noexcept;
			_NODISCARD const int32_t GetWeight() const;
			_NODISCARD const bool CanBlackout() const;
			_NODISCARD const bool DoesTP() const;
			_NODISCARD const bool MatchesRace(const std::vector<RE::Actor*>& list) const;

			void UpdateWeight(const int32_t value);
			void WriteFile();

		public:
			RE::TESQuest* quest;  // is null if requirements arent met

		private:
			const std::string filepath;
			YAML::Node root;
		};

	public:
		_NODISCARD static Resolution* GetSingleton()
		{
			static Resolution singleton;
			return &singleton;
		}

		enum class Type
		{
			Hostile = 0,
			Follower = 1,
			Civilian = 2,
			Guard = 3
		};

	public:
		void Register();
		void UpdateProperties();
		void UpdateWeights();

		RE::TESQuest* SelectQuest(Type type, const std::vector<RE::Actor*>& list, bool blackout);

	private:
		std::map<Type, std::vector<QuestData>> Quests{
			{ Type::Hostile, std::vector<QuestData>{} },
			{ Type::Follower, std::vector<QuestData>{} },
			{ Type::Civilian, std::vector<QuestData>{} },
			{ Type::Guard, std::vector<QuestData>{} }
		};

	private:
		Resolution() = default;
		~Resolution() = default;
		Resolution(const Resolution&) = delete;
		Resolution(Resolution&&) = delete;

		Resolution& operator=(const Resolution&) = delete;
		Resolution& operator=(Resolution&&) = delete;
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
}  // namespace Kudasai