#pragma once

namespace Papyrus
{
	using VM = RE::BSScript::IVirtualMachine;

	// Defeat
	void DefeatActor(RE::StaticFunctionTag*, RE::Actor* subject);
	void RescueActor(RE::StaticFunctionTag*, RE::Actor* subject, bool undo_pacify);
	void PacifyActor(RE::StaticFunctionTag*, RE::Actor* subject);
	void UndoPacify(RE::StaticFunctionTag*, RE::Actor* subject);
	bool IsDefeated(RE::StaticFunctionTag*, RE::Actor* subject);
	bool IsPacified(RE::StaticFunctionTag*, RE::Actor* subject);

	// Actor
	void SetLinkedRef(RE::StaticFunctionTag*, RE::TESObjectREFR* object, RE::TESObjectREFR* target, RE::BGSKeyword* keyword);
	void RemoveAllItems(RE::StaticFunctionTag*, RE::TESObjectREFR*transferfrom, RE::TESObjectREFR*transferto, bool excludeworn, int minvalue);

	// Cofig
	bool ValidRace(RE::StaticFunctionTag*, RE::Actor* subject);
	bool IsInterrested(RE::StaticFunctionTag*, RE::Actor* subject, std::vector<RE::Actor*> partners);

	// Utility
	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::StaticFunctionTag*, RE::Actor* subject, bool ignore_config);

	// Internal
	void SetDamageImmune(RE::StaticFunctionTag*, RE::Actor* subject, bool immune);

	inline bool RegisterFuncs(VM* vm)
	{
		vm->RegisterFunction("DefeatActor", "Kudasai", DefeatActor);
		vm->RegisterFunction("RescueActor", "Kudasai", RescueActor);
		vm->RegisterFunction("PacifyActor", "Kudasai", PacifyActor);
		vm->RegisterFunction("UndoPacify", "Kudasai", UndoPacify);
		vm->RegisterFunction("IsDefeated", "Kudasai", IsDefeated);
		vm->RegisterFunction("IsPacified", "Kudasai", IsPacified);
		vm->RegisterFunction("ValidRace", "Kudasai", ValidRace);
		vm->RegisterFunction("IsInterrested", "Kudasai", IsInterrested);
		vm->RegisterFunction("GetWornArmor", "Kudasai", GetWornArmor);
		vm->RegisterFunction("RemoveAllItems", "Kudasai", RemoveAllItems);
		vm->RegisterFunction("SetLinkedRef", "Kudasai", SetLinkedRef);

		vm->RegisterFunction("SetDamageImmune", "Kudasaiinternal", SetDamageImmune);

		logger::info("Registered Functions");
		return true;
	}
}  // namespace Papyrus
