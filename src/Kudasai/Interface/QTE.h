#pragma once

namespace Kudasai::Interface
{
	class QTE :
		public RE::IMenu,
		public RE::MenuEventHandler
	{
		using ResultHandler = std::function<bool(bool)>;
		using GRefCountBaseStatImpl::operator new;
		using GRefCountBaseStatImpl::operator delete;

	public:
		static constexpr std::string_view NAME{ "QTE" };
		static constexpr std::string_view FILEPATH{ "YameteKudasaiQTE" };

		QTE();
		~QTE() override;
		static void Register();
		static RE::IMenu* CreateQTE() { return new QTE(); }

	public:
		// Menu Controls
		// Opening the Menu is equal to starting the Game. Keys will be taken from the .yaml
		// Set Callback & Timer manually
		static bool IsOpen();
		static void OpenMenu();
		static void CloseMenu();

	public:
		inline static std::vector<int> keys;  // available keys for QTE
		inline static double time;			  // amount time to respond
		inline static ResultHandler handler;  // callback, returns true if another game should start

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
			void Call(Params& a_args) override
			{
				if (!IsOpen())
					return;
				
				bool victory = a_args.args[0].GetBool();
				bool result = handler(victory);
				if (result) {
					auto queue = RE::UIMessageQueue::GetSingleton();
					queue->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kUpdate, nullptr);
				} else {
					logger::info("Closing Menu..");
					CloseMenu();
				}
			}
		};

	private:
		void OnMenuOpen();
		void OnMenuClose();

		void Visible(bool set);

	private:
		RE::GFxValue _main;
	};
}  // namespace Kudasai::Games
