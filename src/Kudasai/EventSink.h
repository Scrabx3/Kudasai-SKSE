#pragma once

#include "Kudasai/Defeat.h"
#include "Kudasai/Animation/Animation.h"

namespace Kudasai
{
	class EventHandler :
		public Singleton<EventHandler>,
		public RE::BSTEventSink<RE::TESDeathEvent>,
		public RE::BSTEventSink<RE::TESCombatEvent>,
		public RE::BSTEventSink<RE::TESObjectLoadedEvent>,
		public RE::BSTEventSink<RE::BSAnimationGraphEvent>
	{
		using EventResult = RE::BSEventNotifyControl;

	public:
		static void Register()
		{
			const auto me = GetSingleton();
			const auto script = RE::ScriptEventSourceHolder::GetSingleton();
			script->AddEventSink<RE::TESObjectLoadedEvent>(me);
			script->AddEventSink<RE::TESDeathEvent>(me);
			script->AddEventSink<RE::TESCombatEvent>(me);
		}

		static void RegisterAnimSink(RE::Actor* subject, const bool add)
		{
			const auto me = GetSingleton();
			if (add)
				subject->AddAnimationGraphEventSink(me);
			else
				subject->RemoveAnimationGraphEventSink(me);
		}

		EventResult ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>*) override
		{
			if (a_event && a_event->holder->Is(RE::FormType::ActorCharacter)) {
				auto source = a_event->holder->As<RE::Actor>();
				if (a_event->tag == "JumpLandEnd" && Defeat::isdefeated(source)) {
					Animation::PlayAnimation(const_cast<RE::Actor*>(source), "BleedoutStart");
				}
			}
			return EventResult::kContinue;
		}

		EventResult ProcessEvent(const RE::TESCombatEvent* a_event, RE::BSTEventSource<RE::TESCombatEvent>*) override
		{
			if (a_event->newState == RE::ACTOR_COMBAT_STATE::kNone || !a_event->targetActor.get()) {
				const auto subject = a_event->actor.get()->As<RE::Actor>();
				if (!Defeat::IsDamageImmune(subject)) {
					const auto ring = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectARMO>(ArmorInvisRing, ESPNAME);
					const auto em = RE::ActorEquipManager::GetSingleton();
					em->EquipObject(subject, ring, nullptr, 1, nullptr, true, true, false);
					em->UnequipObject(subject, ring, nullptr, 1, nullptr, true, false, false);
					subject->RemoveItem(ring, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
				}
			}
			return EventResult::kContinue;
		}

		EventResult ProcessEvent(const RE::TESObjectLoadedEvent* a_event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override
		{
			if (a_event && !a_event->loaded) {
				const auto form = RE::TESForm::LookupByID(a_event->formID);
				if (form && form->Is(RE::FormType::ActorCharacter)) {
					const auto actor = form->As<RE::Actor>();
					if (!actor->IsPlayerTeammate())
						Reset(actor);
				}
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
			SKSE::GetTaskInterface()->AddTask([subject]() {
				if (Defeat::isdefeated(subject)) {
					Defeat::rescue(subject, true, true);
				} else if (Defeat::ispacified(subject)) {
					Defeat::undopacify(subject);
				}
			});
		}
	};
}  // namespace Kudasai
