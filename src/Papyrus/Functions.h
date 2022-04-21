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
	void RemoveAllItems(RE::StaticFunctionTag*, RE::TESObjectREFR* transferfrom, RE::TESObjectREFR* transferto, bool excludeworn);

	// Actor
	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::StaticFunctionTag*, RE::Actor* subject, bool ignore_config);

	// Struggling
	bool CreateStruggle(VM* vm, RE::VMStackID stackID, RE::StaticFunctionTag*, RE::Actor* victim, RE::Actor* aggressor, int difficulty, RE::TESForm* callback);
	void PlayBreakfree(RE::StaticFunctionTag*, std::vector<RE::Actor*> positions);
	void PlayBreakfreeCustom(RE::StaticFunctionTag*, std::vector<RE::Actor*> positions, std::vector<std::string> animations);

	bool IsStruggling(RE::StaticFunctionTag*, RE::Actor* subject);
	bool StopStruggle(RE::StaticFunctionTag*, RE::Actor* victoire);
	bool StopStruggleReverse(RE::StaticFunctionTag*, RE::Actor* defeated);

	// Cofig
	bool ValidRace(RE::StaticFunctionTag*, RE::Actor* subject);
	bool IsInterested(RE::StaticFunctionTag*, RE::Actor* subject, RE::Actor* partners);
	bool IsGroupAllowed(RE::StaticFunctionTag*, RE::Actor* subject, std::vector<RE::Actor*> partners);

	// Internal
	void UpdateWeights(RE::StaticFunctionTag*);

	inline bool RegisterFuncs(VM* vm)
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
		vm->RegisterFunction("CreateStruggle", "Kudasai", CreateStruggle);
		vm->RegisterFunction("PlayBreakfree", "Kudasai", PlayBreakfree);
		vm->RegisterFunction("PlayBreakfreeCustom", "Kudasai", PlayBreakfreeCustom);
		vm->RegisterFunction("IsStruggling", "Kudasai", IsStruggling);
		vm->RegisterFunction("StopStruggle", "Kudasai", StopStruggle);
		vm->RegisterFunction("StopStruggleReverse", "Kudasai", StopStruggleReverse);

		vm->RegisterFunction("UpdateWeights", "KudasaiInternal", UpdateWeights);

		logger::info("Registered Functions");
		return true;
	}
}  // namespace Papyrus
