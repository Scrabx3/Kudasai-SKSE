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
		this->inputContext = Context::kNone;
		this->depthPriority = 3;
		this->menuFlags.set(
			Flag::kUsesMenuContext,
			Flag::kUsesMovementToDirection,
			Flag::kAdvancesUnderPauseMenu,
			Flag::kHasButtonBar,
			Flag::kCustomRendering,
			Flag::kRendersOffscreenTargets,
			Flag::kSkipRenderDuringFreezeFrameScreenshot);

		auto scaleform = RE::BSScaleformManager::GetSingleton();
		bool success = scaleform->LoadMovieEx(this, FILEPATH, [](RE::GFxMovieDef* a_def) -> void {
			a_def->SetState(
				RE::GFxState::StateType::kLog,
				RE::make_gptr<FlashLogger<QTE>>().get());
		});
		if (!success)
			throw InvalidFile();

		auto view = this->uiMovie;
		view->SetMouseCursorCount(0);
		success = view->GetVariable(&_main, "_root.main");
		assert(success && _main.IsObject());

		RE::GFxFunctionHandler* fn = new FlashCallback;
		RE::GFxValue dst;
		view->CreateFunction(&dst, fn);
		success = _main.SetMember("Callback", dst);
		assert(success);

		try {
			YAML::Node root = YAML::LoadFile(CONFIGPATH("Flash.yaml"));
			YAML::Node qte = root["QTE"];
			keys = qte["Keycodes"].as<std::vector<int>>();
			assert(keys.size() > 0);
			YAML::Node ratio = qte["Dimensions"];
			success = _main.SetMember("_minX", RE::GFxValue(ratio["min_X"].as<float>()));
			assert(success);
			success = _main.SetMember("_maxX", RE::GFxValue(ratio["max_X"].as<float>()));
			assert(success);
			success = _main.SetMember("_minY", RE::GFxValue(ratio["min_Y"].as<float>()));
			assert(success);
			success = _main.SetMember("_maxY", RE::GFxValue(ratio["max_Y"].as<float>()));
			assert(success);
		} catch (const std::exception& e) {
			logger::error(e.what());
			keys = std::vector<int>{ 33 };
			success = _main.SetMember("_minX", RE::GFxValue(384));
			assert(success);
			success = _main.SetMember("_maxX", RE::GFxValue(1536));
			assert(success);
			success = _main.SetMember("_minY", RE::GFxValue(216));
			assert(success);
			success = _main.SetMember("_maxY", RE::GFxValue(864));
			assert(success);
		}

		auto mc = RE::MenuControls::GetSingleton();
		mc->RegisterHandler(this);
	}

	QTE::~QTE()
	{
		auto mc = RE::MenuControls::GetSingleton();
		mc->RemoveHandler(this);
	}

	bool QTE::OpenMenu(uint32_t difficulty, CallbackFunc func)
	{
		if (IsOpen())
			return false;

		callback = func;
		time = sqrt(difficulty)/4;
		req_hits = 7 - (difficulty % 5) + Random::draw<short>(0, difficulty % 10);

		RE::UIMessageQueue::GetSingleton()->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
		return true;
	}

	void QTE::CloseMenu(bool force)
	{
		const auto queue = RE::UIMessageQueue::GetSingleton();
		force ? queue->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kForceHide, nullptr) : queue->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
	}

	bool QTE::IsOpen()
	{
		auto ui = RE::UI::GetSingleton();
		return ui->IsMenuOpen(NAME);
	}

	void QTE::Visible(bool set)
	{
		[[maybe_unused]] bool success = _main.SetMember("_visible", RE::GFxValue(set));
		assert(success);
	}

	// IMenu

	RE::UI_MESSAGE_RESULTS QTE::ProcessMessage(RE::UIMessage& a_message)
	{
		using UEFlag = RE::ControlMap::UEFlag;
		using Type = RE::UI_MESSAGE_TYPE;
		using Result = RE::UI_MESSAGE_RESULTS;

		switch (*a_message.type) {
		case Type::kShow:
			Visible(true);
			__fallthrough;
		case Type::kUpdate:
			{
				_lock = false;
				const auto where = Random::draw<size_t>(0, keys.size() - 1);
				std::array<RE::GFxValue, 2>
					args{
						RE::GFxValue(time),
						RE::GFxValue(keys.at(where))
					};
				[[maybe_unused]] bool success = _main.Invoke("CreateGame", args);
				assert(success);
			}
			return Result::kHandled;
		case Type::kForceHide:
			{
				_lock = true;
				[[maybe_unused]] bool success = _main.Invoke("ShutDown");
				assert(success);
			}
			__fallthrough;
		case Type::kHide:
			Visible(false);
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
		else if (a_event->device.any(RE::INPUT_DEVICE::kMouse))
			return false;

		return a_event->eventType.all(Type::kButton);
	}

	bool QTE::ProcessButton(RE::ButtonEvent* a_event)
	{
		const auto& id = a_event->GetIDCode();
		if (id == 1 || id == 15 || id == 28 || id == 197 || id == 199)
			return false;
		if (a_event->IsUp() || a_event->IsRepeating() || _lock)
			return true;
		_lock = true;

		std::array<RE::GFxValue, 1> args{ RE::GFxValue(a_event->GetIDCode()) };
		[[maybe_unused]] bool success = _main.Invoke("KeyDown", args);
		assert(success);
		return true;
	}

	// callback
	void QTE::FlashCallback::Call(Params& a_args)
	{
		bool victory = a_args.args[0].GetBool();
		const auto queue = RE::UIMessageQueue::GetSingleton();
		if (victory && --req_hits > 0) {
			time *= Random::draw<float>(0.7f, 1.25f);
			queue->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kUpdate, nullptr);
			return;
		}
		queue->AddMessage(NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
		callback(victory);
	}
}  // namespace Kudasai::Games