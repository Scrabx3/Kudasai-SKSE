#include "Kudasai/Interface/QTE.h"

#include "Kudasai/Interface/Interface.h"

namespace Kudasai::Interface
{
	void QTE::Register()
	{
		logger::info("Registering QTE Menu");
		const auto ui = RE::UI::GetSingleton();
		ui->Register(NAME, CreateQTE);
	}

	QTE::QTE() :
		RE::IMenu(),
		RE::MenuEventHandler()
	{
		this->inputContext = Context::kGameplay;
		this->depthPriority = 3;
		this->menuFlags.set(
			Flag::kAllowSaving,
			Flag::kRendersOffscreenTargets,
			Flag::kAdvancesUnderPauseMenu,
			Flag::kSkipRenderDuringFreezeFrameScreenshot,
			Flag::kUsesMovementToDirection);

		auto scaleform = RE::BSScaleformManager::GetSingleton();
		bool loaded = scaleform->LoadMovieEx(this, FILEPATH, [](RE::GFxMovieDef* a_def) -> void {
			a_def->SetState(
				RE::GFxState::StateType::kLog,
				RE::make_gptr<FlashLogger<QTE>>().get());
		});
		if (!loaded)
			throw InvalidFile();

		auto view = this->uiMovie;
		view->SetMouseCursorCount(0);
		loaded = view->GetVariable(&_main, "_root.main");
		assert(loaded && _main.IsObject());

		RE::GFxFunctionHandler* fn = new FlashCallback;
		RE::GFxValue dst;
		view->CreateFunction(&dst, fn);
		loaded = _main.SetMember("Hit", dst);
		assert(loaded);

		keys = [](RE::GFxValue& _main) {
			try {
				// Also setting up Widget here, so I only have to read Flash.yaml once per startup
				YAML::Node root = YAML::LoadFile(CONFIGPATH("Flash.yaml"));
				YAML::Node qte = root["QTE"];
				const float ratio = qte["Dimension"].as<float>();
				std::array<RE::GFxValue, 1> args{
					RE::GFxValue(ratio)
				};
				[[maybe_unused]] bool b = _main.Invoke("Prepare", args);
				assert(b);
				return qte["Keycodes"].as<std::vector<int>>();
			} catch (const std::exception& e) {
				// when an error happened, default back to a single "S" key and a ratio of 0.3
				logger::error(e.what());
				std::array<RE::GFxValue, 1> args{
					RE::GFxValue(0.3)
				};
				[[maybe_unused]] bool b = _main.Invoke("Prepare", args);
				assert(b);
				return std::vector<int>{ 31 };
			}
		}(_main);
		assert(keys.size() > 0);

		auto mc = RE::MenuControls::GetSingleton();
		mc->RegisterHandler(this);
	}

	QTE::~QTE()
	{
		auto mc = RE::MenuControls::GetSingleton();
		mc->RemoveHandler(this);
	}

	bool QTE::IsOpen()
	{
		auto ui = RE::UI::GetSingleton();
		return ui->IsMenuOpen(NAME);
	}

	void QTE::OpenMenu()
	{
		auto queue = RE::UIMessageQueue::GetSingleton();
		queue->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
	}

	void QTE::CloseMenu()
	{
		auto queue = RE::UIMessageQueue::GetSingleton();
		queue->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
	}

	void QTE::OnMenuOpen()
	{
		_main.SetMember("_alpha", RE::GFxValue(100));
		// get key to display & invoke creation function
		auto task = SKSE::GetTaskInterface();
		task->AddUITask([this]() {
			const int length = static_cast<int>(keys.size());
			const auto i = randomINT<int>(0, length - 1);
			std::array<RE::GFxValue, 2> args{
				RE::GFxValue(time),
				RE::GFxValue(keys[i])
			};
			logger::info("Creating Game with time = {} and key = {}", time, keys[i]);
			[[maybe_unused]] bool b = _main.Invoke("CreateGame", args);
			assert(b);
		});
	}

	void QTE::OnMenuClose() {}

	// IMenu

	RE::UI_MESSAGE_RESULTS QTE::ProcessMessage(RE::UIMessage& a_message)
	{
		using Type = RE::UI_MESSAGE_TYPE;
		using Result = RE::UI_MESSAGE_RESULTS;
		switch (*a_message.type) {
		case Type::kUpdate:
		case Type::kShow:
			OnMenuOpen();
			return Result::kHandled;
		case Type::kHide:
		case Type::kForceHide:
			OnMenuClose();
			return Result::kHandled;
		default:
			return RE::IMenu::ProcessMessage(a_message);
		}
	}

	// MenuEventHandler

	bool QTE::CanProcess(RE::InputEvent* a_event)
	{
		using Type = RE::INPUT_EVENT_TYPE;
		if (!a_event)
			return false;

		return a_event->eventType.all(Type::kButton);
	}


	bool QTE::ProcessButton(RE::ButtonEvent* a_event)
	{
		if (a_event->IsUp() || a_event->IsRepeating())
			return true;

		auto task = SKSE::GetTaskInterface();
		task->AddUITask([this, a_event]() {
			std::array<RE::GFxValue, 1> args{
				RE::GFxValue(a_event->GetIDCode())
			};
			[[maybe_unused]] bool b = _main.Invoke("KeyDown", args);
			assert(b);
		});
		return true;
	}

	void QTE::Visible(bool set) {
		[[maybe_unused]] bool visible = _main.SetMember("_visible", RE::GFxValue(set));
		assert(visible);
	}

}  // namespace Kudasai::Games