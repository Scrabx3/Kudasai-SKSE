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
		static constexpr std::string_view NAME{ "KudasaiQTE" };
		static constexpr std::string_view FILEPATH{ "YameteKudasaiQTE" };

		QTE();
		~QTE() override;
		static void Register();
		static RE::IMenu* CreateQTE() { return new QTE(); }

	public:
		static bool OpenMenu(uint32_t difficulty, CallbackFunc func);
		static void CloseMenu(bool force);

		static bool IsOpen();

	protected:
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
		void Visible(bool set);

	private:
		static inline CallbackFunc callback;  // callback to invoke when the game is over
		static inline double time;			  // amount time to respond
		static inline short req_hits;		  // hits for the current instance to be completed

		RE::GFxValue _main;			// link to main AS2 Object
		std::vector<int32_t> keys;	// for the QTE allowed Keys. Read on first open

		bool _lock;  // do accept input events?
	};
}  // namespace Kudasai::Games
