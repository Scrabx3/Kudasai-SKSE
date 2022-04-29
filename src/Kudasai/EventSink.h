#pragma once

#include "Kudasai/Defeat.h"

namespace Kudasai
{
	class EventHandler :
		public Singleton<EventHandler>,
		public RE::BSTEventSink<RE::TESDeathEvent>,
		public RE::BSTEventSink<RE::TESObjectLoadedEvent>
	{
		using EventResult = RE::BSEventNotifyControl;

	public:
		static void Register()
		{
			const auto me = GetSingleton();
			const auto script = RE::ScriptEventSourceHolder::GetSingleton();
			script->AddEventSink<RE::TESObjectLoadedEvent>(me);
			script->AddEventSink<RE::TESDeathEvent>(me);
		}

		EventResult ProcessEvent(const RE::TESObjectLoadedEvent* a_event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override
		{
			if (a_event && !a_event->loaded) {
				const auto form = RE::TESForm::LookupByID(a_event->formID);
				if (form && form->Is(RE::FormType::ActorCharacter))
					Reset(form->As<RE::Actor>());
			}
			return EventResult::kContinue;	
		}

		EventResult ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*) override
		{
			if (a_event && a_event->actorDying) {
				const auto form = a_event->actorDying.get();
				if (form && form->Is(RE::FormType::ActorCharacter))
					Reset(form->As<RE::Actor>());
			}
			return EventResult::kContinue;
		}

		void Reset(RE::Actor* subject)
		{
			if (Defeat::isdefeated(subject)) {
				Defeat::RescueImpl(subject);
				Defeat::UndoPacifyImpl(subject);
			} else if (Defeat::ispacified(subject)) {
				Defeat::UndoPacifyImpl(subject);
			}
		}
	};
}  // namespace Kudasai
