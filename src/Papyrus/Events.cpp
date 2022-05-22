#include "Papyrus/Events.h"

#include "Serialization/EventManager.h"

namespace Papyrus
{
	void RegisterForActorDefeated(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* form)
	{
		if (!form) {
			a_vm->TraceStack("Form is none", a_stackID);
			return;
		}
		Serialization::EventManager::GetSingleton()->_actordefeated.Register(form);
	}
	void UnregisterForActorDefeated(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* form)
	{
		if (!form) {
			a_vm->TraceStack("Form is none", a_stackID);
			return;
		}
		Serialization::EventManager::GetSingleton()->_actordefeated.Unregister(form);
	}
	void RegisterForActorDefeated_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* alias)
	{
		if (!alias) {
			a_vm->TraceStack("Form is none", a_stackID);
			return;
		}
		Serialization::EventManager::GetSingleton()->_actordefeated.Register(alias);
	}
	void UnregisterForActorDefeated_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* alias)
	{
		if (!alias) {
			a_vm->TraceStack("Form is none", a_stackID);
			return;
		}
		Serialization::EventManager::GetSingleton()->_actordefeated.Unregister(alias);
	}
	void RegisterForActorDefeated_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::ActiveEffect* activeeffect)
	{
		if (!activeeffect) {
			a_vm->TraceStack("Form is none", a_stackID);
			return;
		}
		Serialization::EventManager::GetSingleton()->_actordefeated.Register(activeeffect);
	}
	void UnregisterForActorDefeated_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::ActiveEffect* activeeffect)
	{
		if (!activeeffect) {
			a_vm->TraceStack("Form is none", a_stackID);
			return;
		}
		Serialization::EventManager::GetSingleton()->_actordefeated.Unregister(activeeffect);
	}

	void RegisterForActorRescued(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* form)
	{
		if (!form) {
			a_vm->TraceStack("Form is none", a_stackID);
			return;
		}
		Serialization::EventManager::GetSingleton()->_actorrescued.Register(form);
	}
	void UnregisterForActorRescued(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* form)
	{
		if (!form) {
			a_vm->TraceStack("Form is none", a_stackID);
			return;
		}
		Serialization::EventManager::GetSingleton()->_actorrescued.Unregister(form);
	}
	void RegisterForActorRescued_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* alias)
	{
		if (!alias) {
			a_vm->TraceStack("Form is none", a_stackID);
			return;
		}
		Serialization::EventManager::GetSingleton()->_actorrescued.Register(alias);
	}
	void UnregisterForActorRescued_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* alias)
	{
		if (!alias) {
			a_vm->TraceStack("Form is none", a_stackID);
			return;
		}
		Serialization::EventManager::GetSingleton()->_actorrescued.Unregister(alias);
	}
	void RegisterForActorRescued_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::ActiveEffect* activeeffect)
	{
		if (!activeeffect) {
			a_vm->TraceStack("Form is none", a_stackID);
			return;
		}
		Serialization::EventManager::GetSingleton()->_actorrescued.Register(activeeffect);
	}
	void UnregisterForActorRescued_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::ActiveEffect* activeeffect)
	{
		if (!activeeffect) {
			a_vm->TraceStack("Form is none", a_stackID);
			return;
		}
		Serialization::EventManager::GetSingleton()->_actorrescued.Unregister(activeeffect);
	}

}  // namespace Papyrus
