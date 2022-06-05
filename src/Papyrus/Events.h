#pragma once

#include "Serialization/EventManager.h"

namespace Papyrus
{
	using VM = RE::BSScript::IVirtualMachine;

#define REGISTER(type, thing)                        \
	if (!thing) {                                    \
		a_vm->TraceStack("Form is none", a_stackID); \
		return;                                      \
	}                                                \
	Serialization::EventManager::GetSingleton()->type.Register(thing);
#define UNREGISTER(type, thing)                      \
	if (!thing) {                                    \
		a_vm->TraceStack("Form is none", a_stackID); \
		return;                                      \
	}                                                \
	Serialization::EventManager::GetSingleton()->type.Unregister(thing);


	inline void RegisterForActorDefeated(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* form)
	{
		REGISTER(_actordefeated, form);
	}
	inline void UnregisterForActorDefeated(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* form)
	{
		UNREGISTER(_actordefeated, form);
	}
	inline void RegisterForActorDefeated_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::BGSRefAlias* alias)
	{
		REGISTER(_actordefeated, alias);
	}
	inline void UnregisterForActorDefeated_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::BGSRefAlias* alias)
	{
		UNREGISTER(_actordefeated, alias);
	}
	inline void RegisterForActorDefeated_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::ActiveEffect* activeEffect)
	{
		REGISTER(_actordefeated, activeEffect);
	}
	inline void UnregisterForActorDefeated_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::ActiveEffect* activeEffect)
	{
		UNREGISTER(_actordefeated, activeEffect);
	}

	inline void RegisterForActorRescued(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* form)
	{
		REGISTER(_actorrescued, form);
	}
	inline void UnregisterForActorRescued(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* form)
	{
		UNREGISTER(_actorrescued, form);
	}
	inline void RegisterForActorRescued_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::BGSRefAlias* alias)
	{
		REGISTER(_actorrescued, alias);
	}
	inline void UnregisterForActorRescued_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::BGSRefAlias* alias)
	{
		UNREGISTER(_actorrescued, alias);
	}
	inline void RegisterForActorRescued_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::ActiveEffect* activeEffect)
	{
		REGISTER(_actorrescued, activeEffect);
	}
	inline void UnregisterForActorRescued_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::ActiveEffect* activeEffect)
	{
		UNREGISTER(_actorrescued, activeEffect);
	}


	inline bool RegisterEvents(VM* vm)
	{
		vm->RegisterFunction("RegisterForActorDefeated", "Kudasai", RegisterForActorDefeated, true);
		vm->RegisterFunction("UnregisterForActorDefeated", "Kudasai", UnregisterForActorDefeated, true);
		vm->RegisterFunction("RegisterForActorDefeated_Alias", "Kudasai", RegisterForActorDefeated_Alias, true);
		vm->RegisterFunction("UnregisterForActorDefeated_Alias", "Kudasai", UnregisterForActorDefeated_Alias, true);
		vm->RegisterFunction("RegisterForActorDefeated_MgEff", "Kudasai", RegisterForActorDefeated_MgEff, true);
		vm->RegisterFunction("UnregisterForActorDefeated_MgEff", "Kudasai", UnregisterForActorDefeated_MgEff, true);

		vm->RegisterFunction("RegisterForActorRescued", "Kudasai", RegisterForActorRescued, true);
		vm->RegisterFunction("UnregisterForActorRescued", "Kudasai", UnregisterForActorRescued, true);
		vm->RegisterFunction("RegisterForActorRescued_Alias", "Kudasai", RegisterForActorRescued_Alias, true);
		vm->RegisterFunction("UnregisterForActorRescued_Alias", "Kudasai", UnregisterForActorRescued_Alias, true);
		vm->RegisterFunction("RegisterForActorRescued_MgEff", "Kudasai", RegisterForActorRescued_MgEff, true);
		vm->RegisterFunction("UnregisterForActorRescued_MgEff", "Kudasai", UnregisterForActorRescued_MgEff, true);

		return true;
	}
} // namespace Papyrus

