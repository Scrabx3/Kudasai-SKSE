#pragma once

namespace Papyrus
{
	using VM = RE::BSScript::IVirtualMachine;

	void RegisterForActorDefeated(RE::StaticFunctionTag*, const RE::TESForm* form);
	void UnregisterForActorDefeated(RE::StaticFunctionTag*, const RE::TESForm* form);
	void RegisterForActorDefeated_Alias(RE::StaticFunctionTag*, RE::BGSBaseAlias* alias);
	void UnregisterForActorDefeated_Alias(RE::StaticFunctionTag*, RE::BGSBaseAlias* alias);
	void RegisterForActorDefeated_MgEff(RE::StaticFunctionTag*, RE::ActiveEffect* activeEffect);
	void UnregisterForActorDefeated_MgEff(RE::StaticFunctionTag*, RE::ActiveEffect* activeEffect);

	void RegisterForActorRescued(RE::StaticFunctionTag*, const RE::TESForm* form);
	void UnregisterForActorRescued(RE::StaticFunctionTag*, const RE::TESForm* form);
	void RegisterForActorRescued_Alias(RE::StaticFunctionTag*, RE::BGSBaseAlias* alias);
	void UnregisterForActorRescued_Alias(RE::StaticFunctionTag*, RE::BGSBaseAlias* alias);
	void RegisterForActorRescued_MgEff(RE::StaticFunctionTag*, RE::ActiveEffect* activeEffect);
	void UnregisterForActorRescued_MgEff(RE::StaticFunctionTag*, RE::ActiveEffect* activeEffect);


	inline bool RegisterEvents(VM* vm)
	{
		vm->RegisterFunction("RegisterForActorDefeated", "KudasaiEvents", RegisterForActorDefeated, true);
		vm->RegisterFunction("UnregisterForActorDefeated", "KudasaiEvents", UnregisterForActorDefeated, true);
		vm->RegisterFunction("RegisterForActorDefeated_Alias", "KudasaiEvents", RegisterForActorDefeated_Alias, true);
		vm->RegisterFunction("UnregisterForActorDefeated_Alias", "KudasaiEvents", UnregisterForActorDefeated_Alias, true);
		vm->RegisterFunction("RegisterForActorDefeated_MgEff", "KudasaiEvents", RegisterForActorDefeated_MgEff, true);
		vm->RegisterFunction("UnregisterForActorDefeated_MgEff", "KudasaiEvents", UnregisterForActorDefeated_MgEff, true);

		vm->RegisterFunction("RegisterForActorRescued", "KudasaiEvents", RegisterForActorRescued, true);
		vm->RegisterFunction("UnregisterForActorRescued", "KudasaiEvents", UnregisterForActorRescued, true);
		vm->RegisterFunction("RegisterForActorRescued_Alias", "KudasaiEvents", RegisterForActorRescued_Alias, true);
		vm->RegisterFunction("UnregisterForActorRescued_Alias", "KudasaiEvents", UnregisterForActorRescued_Alias, true);
		vm->RegisterFunction("RegisterForActorRescued_MgEff", "KudasaiEvents", RegisterForActorRescued_MgEff, true);
		vm->RegisterFunction("UnregisterForActorRescued_MgEff", "KudasaiEvents", UnregisterForActorRescued_MgEff, true);

		return true;
	}
} // namespace Papyrus

