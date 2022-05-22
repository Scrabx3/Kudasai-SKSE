#pragma once

namespace Papyrus
{
	using VM = RE::BSScript::IVirtualMachine;

	void RegisterForActorDefeated(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* form);
	void UnregisterForActorDefeated(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* form);
	void RegisterForActorDefeated_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* alias);
	void UnregisterForActorDefeated_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* alias);
	void RegisterForActorDefeated_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::ActiveEffect* activeEffect);
	void UnregisterForActorDefeated_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::ActiveEffect* activeEffect);

	void RegisterForActorRescued(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* form);
	void UnregisterForActorRescued(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* form);
	void RegisterForActorRescued_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* alias);
	void UnregisterForActorRescued_Alias(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* alias);
	void RegisterForActorRescued_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::ActiveEffect* activeEffect);
	void UnregisterForActorRescued_MgEff(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, const RE::ActiveEffect* activeEffect);


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

