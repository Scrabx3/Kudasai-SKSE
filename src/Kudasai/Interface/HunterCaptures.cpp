#include "Kudasai/Interface/HunterCaptures.h"

#include "Kudasai/Interface/Interface.h"

namespace Kudasai::Interface
{
	void HunterCaptures::Register()
	{
		logger::info("Registering HunterCaptures Menu");
		const auto ui = RE::UI::GetSingleton();
		ui->Register(NAME, CreateMenu);
	}

	HunterCaptures::HunterCaptures() :
		RE::IMenu()
	{
		this->inputContext = Context::kMenuMode;
		this->depthPriority = 3;
		this->menuFlags.set(
			Flag::kPausesGame,
			Flag::kUsesMenuContext,
			Flag::kModal,
			Flag::kUsesCursor,
			Flag::kDisablePauseMenu,
			Flag::kInventoryItemMenu,
			Flag::kCustomRendering,
			Flag::kApplicationMenu);

		auto scaleform = RE::BSScaleformManager::GetSingleton();
		bool success = scaleform->LoadMovieEx(this, FILEPATH, [](RE::GFxMovieDef* a_def) -> void {
			a_def->SetState(
				RE::GFxState::StateType::kLog,
				RE::make_gptr<FlashLogger<HunterCaptures>>().get());
		});
		if (!success)
			throw InvalidFile();

		auto view = this->uiMovie;
		view->SetMouseCursorCount(0);
	}

	bool HunterCaptures::OpenMenu()
	{
		if (IsOpen())
			return false;
  
		RE::UIMessageQueue::GetSingleton()->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
		return true;
	}

	void HunterCaptures::CloseMenu()
	{
		const auto queue = RE::UIMessageQueue::GetSingleton();
		queue->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
	}

	bool HunterCaptures::IsOpen()
	{
		auto ui = RE::UI::GetSingleton();
		return ui->IsMenuOpen(NAME);
	}

	RE::UI_MESSAGE_RESULTS HunterCaptures::ProcessMessage(RE::UIMessage& a_message)
	{
		using Type = RE::UI_MESSAGE_TYPE;
		using Result = RE::UI_MESSAGE_RESULTS;

		switch (*a_message.type) {
		case Type::kShow:
			{
				RE::GFxValue _main;
				auto success = this->uiMovie->GetVariable(&_main, "_root.main");
				assert(success && _main.IsObject());
        // gonna use the global as upper limit here so I dont have to adjust code here if I ever change the limit in papyrus
				const auto capacity = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(GlobalCapturesCapacity, ESPNAME);
				for (size_t i = 0; i < capacity->value; i++) {
          
        }
        
        // TODO: Update .txt properties
        _main.Invoke("OpenMenu");
			}
		case Type::kHide:
			return Result::kHandled;
		default:
			return RE::IMenu::ProcessMessage(a_message);
		}
	}
}  // namespace Kudasai::Interface
