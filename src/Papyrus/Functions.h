#pragma once

namespace Papyrus
{
	using VM = RE::BSScript::IVirtualMachine;
	using StackID = RE::VMStackID;

	// Defeat
	void DefeatActor(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, bool skip_animation);
	void RescueActor(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, bool undo_pacify, bool skip_animation);
	void PacifyActor(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
	void UndoPacify(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
	bool IsDefeated(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
	bool IsPacified(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);

	// ObjectReference
	void SetLinkedRef(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* object, RE::TESObjectREFR* target, RE::BGSKeyword* keyword);
	void RemoveAllItems(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* transferfrom, RE::TESObjectREFR* transferto, bool excludeworn);

	// Actor
	std::vector<RE::TESObjectARMO*> GetWornArmor(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, bool ignore_config);
	RE::AlchemyItem* GetMostEfficientPotion(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, RE::TESObjectREFR* container);
	RE::TESNPC* GetTemplateBase(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* akActor);
	std::string GetRaceKey(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* akActor);

	// Cofig
	bool ValidRace(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
	bool IsInterested(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, RE::Actor* partners);
	// bool IsGroupAllowed(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, std::vector<RE::Actor*> partners);

	// Utility
	void RemoveArmorByKeyword(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::TESObjectARMO*> array, RE::BGSKeyword* keyword);
	void CreateFuture(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, float duration, RE::TESForm* callback, std::vector<RE::Actor*> argActor, int32_t argNum, RE::BSFixedString argStr);

	// Internal
	void UpdateSettings(RE::StaticFunctionTag*);

	// Struggle
	std::vector<std::string> LookupStruggleAnimations(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions);
	std::vector<std::string> LookupBreakfreeAnimations(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions);
	std::vector<std::string> LookupKnockoutAnimations(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions);
	void SetPositions(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions);
	void ClearPositions(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions);

	bool OpenQTEMenu(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, int32_t difficulty, RE::TESForm* callback);
	void CloseQTEMenu(RE::StaticFunctionTag*);

	inline bool IsAlternateVersion(RE::StaticFunctionTag*) { return !Kudasai::IsLight(); }

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
		vm->RegisterFunction("GetWornArmor", "Kudasai", GetWornArmor);
		vm->RegisterFunction("RemoveAllItems", "Kudasai", RemoveAllItems);
		vm->RegisterFunction("SetLinkedRef", "Kudasai", SetLinkedRef);
		vm->RegisterFunction("GetMostEfficientPotion", "Kudasai", GetMostEfficientPotion);
		vm->RegisterFunction("RemoveArmorByKeyword", "Kudasai", RemoveArmorByKeyword);
		vm->RegisterFunction("GetTemplateBase", "Kudasai", GetTemplateBase);
		vm->RegisterFunction("CreateFuture", "Kudasai", CreateFuture);
		vm->RegisterFunction("GetRaceKey", "Kudasai", GetRaceKey);

		vm->RegisterFunction("UpdateSettings", "KudasaiInternal", UpdateSettings);
		vm->RegisterFunction("IsAlternateVersion", "KudasaiInternal", IsAlternateVersion);

		vm->RegisterFunction("LookupStruggleAnimations", "KudasaiStruggle", LookupStruggleAnimations);
		vm->RegisterFunction("LookupBreakfreeAnimations", "KudasaiStruggle", LookupBreakfreeAnimations);
		vm->RegisterFunction("LookupKnockoutAnimations", "KudasaiStruggle", LookupKnockoutAnimations);
		vm->RegisterFunction("SetPositions", "KudasaiStruggle", SetPositions);
		vm->RegisterFunction("ClearPositions", "KudasaiStruggle", ClearPositions);
		vm->RegisterFunction("OpenQTEMenu", "KudasaiStruggle", OpenQTEMenu);
		vm->RegisterFunction("CloseQTEMenu", "KudasaiStruggle", CloseQTEMenu);

		logger::info("Registered Functions");
		return true;
	}
}  // namespace Papyrus
