#pragma once

#include "Kudasai/Interface/Interface.h"

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
		static bool OpenMenu(int option0, int option1, int option2, int option3, int option4, int option5, int option6, int option7, int option8, int option9);
		static void CloseMenu();

		static bool IsOpen();

	protected:
		// IMenu
		RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override;

	private:
		RE::GFxValue _main;
  };


  void HunterPride::Register()
  {
	  logger::info("Registering HunterPride Menu");
	  const auto ui = RE::UI::GetSingleton();
	  ui->Register(NAME, CreateMenu);
  }

  HunterPride::HunterPride() :
	  RE::IMenu()
  {
	  this->inputContext = Context::kMenuMode;
	  this->depthPriority = 3;
	  this->menuFlags.set(
		  Flag::kPausesGame,
		  Flag::kUsesMenuContext,
		  Flag::kModal,
		  Flag::kDisablePauseMenu,
		  Flag::kInventoryItemMenu,
		  Flag::kCustomRendering,
		  Flag::kApplicationMenu);

	  auto scaleform = RE::BSScaleformManager::GetSingleton();
	  bool success = scaleform->LoadMovieEx(this, FILEPATH, [](RE::GFxMovieDef* a_def) -> void {
		  a_def->SetState(
			  RE::GFxState::StateType::kLog,
			  RE::make_gptr<FlashLogger<HunterPride>>().get());
	  });
	  if (!success)
		  throw InvalidFile();

	  auto view = this->uiMovie;
	  view->SetMouseCursorCount(0);
	  success = view->GetVariable(&_main, "_root.YamMenu");
	  assert(success && _main.IsObject());
  }

  bool HunterPride::OpenMenu(int option0, int option1, int option2, int option3, int option4, int option5, int option6, int option7, int option8, int option9)
  {
	  if (IsOpen())
		  return false;		

	  RE::UIMessageQueue::GetSingleton()->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
	  return true;
  }

  void HunterPride::CloseMenu()
  {
	  const auto queue = RE::UIMessageQueue::GetSingleton();
	  queue->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
  }

  bool HunterPride::IsOpen()
  {
	  auto ui = RE::UI::GetSingleton();
	  return ui->IsMenuOpen(NAME);
  }
}