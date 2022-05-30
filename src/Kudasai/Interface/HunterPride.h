#pragma once

namespace Kudasai::Interface
{
	class HunterPride :
		public RE::IMenu
	{
		using GRefCountBaseStatImpl::operator new;
		using GRefCountBaseStatImpl::operator delete;

	public:
		static constexpr std::string_view NAME{ "KudasaiHunterPride" };
		static constexpr std::string_view FILEPATH{ "YameteKudasaiHunterPride" };

		HunterPride();
		static void Register();
		static RE::IMenu* CreateMenu() { return new HunterPride(); }
	
	public:
		static bool OpenMenu(std::vector<int32_t> a_options);
		static void CloseMenu();

		static bool IsOpen();

	protected:
		// IMenu
		RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override;

	private:
		inline static std::vector<int32_t> options;
  };
}