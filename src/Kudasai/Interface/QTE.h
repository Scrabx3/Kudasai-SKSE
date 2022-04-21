#pragma once

namespace Kudasai::Interface
{
	class QTE :
		public RE::IMenu,
		public RE::MenuEventHandler
	{
		using CallbackFunc = std::function<void(bool)>;
		using GRefCountBaseStatImpl::operator new;
		using GRefCountBaseStatImpl::operator delete;

	public:
		enum class Difficulty
		{
			Easy = 0,
			Normal = 1,
			Hard = 2,
			Legendary = 3
		};

		static constexpr std::string_view NAME{ "QTE" };
		static constexpr std::string_view FILEPATH{ "YameteKudasaiQTE" };

		QTE();
		~QTE() override;
		static void Register();
		static RE::IMenu* CreateQTE() { return new QTE(); }

	public:
		/**
		 * @brief Start a new Game
		 * 
		 * @param difficulty The difficulty of the Game
		 * @param callback void(bool), Invoked after the game ended, carrying the result (true = victory)
		 */
		static bool CreateGame(Difficulty difficulty, CallbackFunc func);
		static bool IsOpen();

		static void CloseMenu(bool force);

	public:
		// IMenu
		RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override;

		// MenuEventHandler
		bool CanProcess(RE::InputEvent* a_event) override;
		bool ProcessButton(RE::ButtonEvent* a_event) override;

		// Scaleform Callback
		class FlashCallback : public RE::GFxFunctionHandler
		{
		public:
			void Call(Params& a_args) override;
		};

	private:
		void Display();
		
		bool Visible();
		void Visible(bool set);

	private:
		static inline CallbackFunc callback;	// callback to invoke when the game is over
		static inline double time;			// amount time to respond
		static inline int requiredhits;		// number of times to respond

		RE::GFxValue _main;			// link to main AS2 Object
		std::vector<int32_t> keys;	// for the QTE allowed Keys

		bool running;  // if a game is currently running
	};
}  // namespace Kudasai::Games
