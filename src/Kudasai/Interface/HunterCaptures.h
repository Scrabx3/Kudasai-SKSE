#pragma once

namespace Kudasai::Interface
{
	class HunterCaptures :
		public RE::IMenu
	{
		using GRefCountBaseStatImpl::operator new;
		using GRefCountBaseStatImpl::operator delete;

	public:
		static constexpr std::string_view NAME{ "KudasaiHunterCaptures" };
		static constexpr std::string_view FILEPATH{ "YameteKudasaiHunterCaptures" };

		HunterCaptures();
		static void Register();
		static RE::IMenu* CreateMenu() { return new HunterCaptures(); }

	public:
    static void CollectData(int idx, RE::Actor* subject);
		static bool OpenMenu();
		static void CloseMenu();

		static bool IsOpen();

	protected:
		// IMenu
		RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override;

	private:
    struct Data
    {
      int32_t idx;

      std::string name;
      RE::SEX sex;
      uint32_t level;
      std::string location;
      std::string time;
      uint32_t rarity;
      bool wanted;
    };

		inline static std::vector<Data> data;
	};
}