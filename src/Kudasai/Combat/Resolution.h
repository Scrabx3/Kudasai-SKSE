namespace Kudasai
{

	class Resolution :
		public Singleton<Resolution>
	{
		struct QuestData
		{
		public:
			QuestData(const std::string filepath);
			~QuestData() = default;

			_NODISCARD std::string GetName() const { return GetAttribute("Name", "_UNDEFINED"s); }
			_NODISCARD std::string GetDescription() const { return GetAttribute("Description", ""s); }
			_NODISCARD bool IsBlackout() const { return GetAttribute("IsBlackout", false); }
			_NODISCARD bool IsTeleport() const { return GetAttribute("IsTeleport", true); }
			_NODISCARD bool IsHidden() const { return GetAttribute("IsHidden", false); }
			_NODISCARD int GetWeight() const { return GetAttribute("Weight", 50); }

			_NODISCARD int ValidateRequirements(const std::vector<RE::Actor*>& list) const;

			void UpdateWeight(int32_t value) { root["Weight"] = value; }
			void WriteFile() { std::ofstream{ filepath } << root; }

		public:
			RE::TESQuest* quest;  // is null if requirements arent met

		private:
			template <class T>
			T GetAttribute(const char* a_attribute, T a_missing) const
			{
				if (root[a_attribute].IsDefined())
					return root[a_attribute].as<T>();
				logger::warn("Event {}: \"{}\" not defined. Assuming {}", GetName(), a_attribute, a_missing);
				return a_missing;
			}

		private:
			const std::string filepath;
			YAML::Node root;
		};

	public:
		enum Type
		{
			Hostile = 0,
			Follower = 1,
			Civilian = 2,
			Guard = 3,

			Total = 4
		};

	public:
		void Register();
		void UpdateProperties();
		void UpdateWeights();

		RE::TESQuest* SelectQuest(Type type, const std::vector<RE::Actor*>& members, bool blackout);

	private:
		std::vector<QuestData> Quests[Type::Total];

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