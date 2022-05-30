#include "Kudasai/Interface/HunterPride.h"

#include "Kudasai/Interface/Interface.h"

namespace Kudasai::Interface
{
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
			// Flag::kPausesGame,
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
				RE::make_gptr<FlashLogger<HunterPride>>().get());
		});
		if (!success)
			throw InvalidFile();

		auto view = this->uiMovie;
		view->SetMouseCursorCount(0);
	}

	bool HunterPride::OpenMenu(std::vector<int32_t> a_options)
	{
		if (IsOpen())
			return false;
		
		options = a_options;
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

	RE::UI_MESSAGE_RESULTS HunterPride::ProcessMessage(RE::UIMessage& a_message)
	{
		using Type = RE::UI_MESSAGE_TYPE;
		using Result = RE::UI_MESSAGE_RESULTS;

		switch (*a_message.type) {
		case Type::kShow:
			{
				RE::GFxValue _main;
				auto success = this->uiMovie->GetVariable(&_main, "_root.YamMenu");
				assert(success && _main.IsObject());
				std::array<RE::GFxValue, 10> args{
					RE::GFxValue(options[0]),
					RE::GFxValue(options[1]),
					RE::GFxValue(options[2]),
					RE::GFxValue(options[3]),
					RE::GFxValue(options[4]),
					RE::GFxValue(options[5]),
					RE::GFxValue(options[6]),
					RE::GFxValue(options[7]),
					RE::GFxValue(options[8]),
					RE::GFxValue(options[9])
				};
				_main.Invoke("OpenMenu", args);
			}
		case Type::kHide:
			return Result::kHandled;
		default:
			return RE::IMenu::ProcessMessage(a_message);
		}
	}
} // namespace Kudasai::Interface
