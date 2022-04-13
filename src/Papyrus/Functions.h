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

	// ObjectReference
	void SetLinkedRef(RE::StaticFunctionTag*, RE::TESObjectREFR* object, RE::TESObjectREFR* target, RE::BGSKeyword* keyword);
	void RemoveAllItems(RE::StaticFunctionTag*, RE::TESObjectREFR* transferfrom, RE::TESObjectREFR* transferto, bool excludeworn, int minvalue);

	// Actor
	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::StaticFunctionTag*, RE::Actor* subject, bool ignore_config);

	// Cofig
	bool ValidRace(RE::StaticFunctionTag*, RE::Actor* subject);
	bool IsInterested(RE::StaticFunctionTag*, RE::Actor* subject, RE::Actor* partners);
	bool IsGroupAllowed(RE::StaticFunctionTag*, RE::Actor* subject, std::vector<RE::Actor*> partners);

	// Internal
	void SetDamageImmune(RE::StaticFunctionTag*, RE::Actor* subject, bool immune);

	// Internal - MCM
	void UpdateWeights(RE::TESQuest*);

	inline bool	RegisterFuncs(VM* vm)
	{
		vm->RegisterFunction("DefeatActor", "Kudasai", DefeatActor);
		vm->RegisterFunction("RescueActor", "Kudasai", RescueActor);
		vm->RegisterFunction("PacifyActor", "Kudasai", PacifyActor);
		vm->RegisterFunction("UndoPacify", "Kudasai", UndoPacify);
		vm->RegisterFunction("IsDefeated", "Kudasai", IsDefeated);
		vm->RegisterFunction("IsPacified", "Kudasai", IsPacified);
		vm->RegisterFunction("ValidRace", "Kudasai", ValidRace);
		vm->RegisterFunction("IsInterested", "Kudasai", IsInterested);
		vm->RegisterFunction("IsGroupAllowed", "Kudasai", IsGroupAllowed);
		vm->RegisterFunction("GetWornArmor", "Kudasai", GetWornArmor);
		vm->RegisterFunction("RemoveAllItems", "Kudasai", RemoveAllItems);
		vm->RegisterFunction("SetLinkedRef", "Kudasai", SetLinkedRef);

		vm->RegisterFunction("SetDamageImmune", "KudasaiInternal", SetDamageImmune);

		vm->RegisterFunction("UpdateWeights", "KudasaiMCM", UpdateWeights);

		logger::info("Registered Functions");
		return true;
	}
}  // namespace Papyrus
