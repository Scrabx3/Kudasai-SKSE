#pragma once

#include "Kudasai/Animation/Animation.h"
#include "Kudasai/Defeat.h"

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

		std::map<RE::FormID, std::vector<RE::TESObjectARMO*>> worn_cache{};

	protected:
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
			const auto subject = a_event->actor.get()->As<RE::Actor>();
			if (subject && subject->Is3DLoaded() && !subject->IsDead()) {
				if (a_event->newState == RE::ACTOR_COMBAT_STATE::kNone) {
					if (!Defeat::IsDamageImmune(subject)) {
						auto where = worn_cache.find(subject->GetFormID());
						if (where != worn_cache.end()) {
							const auto em = RE::ActorEquipManager::GetSingleton();
							for (auto& e : where->second) {
								if (!subject->GetWornArmor(e->GetSlotMask())) {
									em->EquipObject(subject, e);
								}
							}
							worn_cache.erase(where);
						}
					}
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
					worn_cache.erase(form->GetFormID());
					if (!actor->IsPlayerTeammate()) {
						Reset(actor);
					}
				}
			}
			return EventResult::kContinue;
		}

		EventResult ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*) override
		{
			if (a_event && a_event->actorDying) {
				const auto form = a_event->actorDying.get();
				if (form && form->Is(RE::FormType::ActorCharacter)) {
					worn_cache.erase(form->GetFormID());
					Reset(form->As<RE::Actor>());
				}
			}
			return EventResult::kContinue;
		}

	private:
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
