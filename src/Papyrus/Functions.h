#pragma once

namespace Papyrus
{
#define REGISTERFUNC(func, c) a_vm->RegisterFunction(#func##sv, c, func)

	using VM = RE::BSScript::IVirtualMachine;
	using StackID = RE::VMStackID;

	namespace Defeat
	{
		void DefeatActor(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, bool skip_animation);
		void RescueActor(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, bool undo_pacify, bool skip_animation);
		void PacifyActor(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
		void UndoPacify(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
		bool IsDefeated(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
		bool IsPacified(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);

		std::vector<RE::Actor*> GetDefeated(RE::StaticFunctionTag*);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(DefeatActor, "Kudasai");
			REGISTERFUNC(RescueActor, "Kudasai");
			REGISTERFUNC(PacifyActor, "Kudasai");
			REGISTERFUNC(UndoPacify, "Kudasai");
			REGISTERFUNC(IsDefeated, "Kudasai");
			REGISTERFUNC(IsPacified, "Kudasai");
			REGISTERFUNC(GetDefeated, "Kudasai");
		}
	}

	namespace ObjectRef
	{
		void SetLinkedRef(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* object, RE::TESObjectREFR* target, RE::BGSKeyword* keyword);
		void RemoveAllItems(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* transferfrom, RE::TESObjectREFR* transferto, bool excludeworn);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(SetLinkedRef, "Kudasai");
			REGISTERFUNC(RemoveAllItems, "Kudasai");
		}
	}

	namespace Actor
	{
		std::vector<RE::TESObjectARMO*> GetWornArmor(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
		std::vector<RE::TESObjectARMO*> StripActor(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, int32_t ignoredmasks);
		RE::AlchemyItem* GetMostEfficientPotion(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, RE::TESObjectREFR* container);
		RE::TESNPC* GetTemplateBase(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* akActor);
		std::string GetRaceType(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* akActor);
		std::vector<RE::Actor*> GetFollowers(RE::StaticFunctionTag*);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(GetWornArmor, "Kudasai");
			REGISTERFUNC(StripActor, "Kudasai");
			REGISTERFUNC(GetMostEfficientPotion, "Kudasai");
			REGISTERFUNC(GetTemplateBase, "Kudasai");
			REGISTERFUNC(GetRaceType, "Kudasai");
			REGISTERFUNC(GetFollowers, "Kudasai");
		}
	}

	namespace Config
	{
		bool ValidRace(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject);
		bool IsInterested(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, RE::Actor* partners);
		// bool IsGroupAllowed(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::Actor* subject, std::vector<RE::Actor*> partners);

		void UpdateSettings(RE::StaticFunctionTag*);
		bool IsAlternateVersion(RE::StaticFunctionTag*);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(ValidRace, "Kudasai");
			REGISTERFUNC(IsInterested, "Kudasai");
			REGISTERFUNC(UpdateSettings, "KudasaiInternal");
			REGISTERFUNC(IsAlternateVersion, "KudasaiInternal");
		}
	}

	namespace Utility
	{
		void DisableProcessing(RE::StaticFunctionTag*, bool disable);
		bool IsProcessingDisabled(RE::StaticFunctionTag*);
		void DisableConsequence(RE::StaticFunctionTag*, bool disable);
		bool IsConsequenceDisabled(RE::StaticFunctionTag*);
		std::vector<RE::TESObjectARMO*> RemoveArmorByKeyword(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::TESObjectARMO*> array, RE::BGSKeyword* keyword);
		void CreateFuture(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, float duration, RE::TESForm* callback, std::vector<RE::Actor*> argActor, int32_t argNum, RE::BSFixedString argStr);
		void SortByDistance(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> array, RE::TESObjectREFR* center);
		void SortByDistanceRef(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> array, RE::TESObjectREFR* center);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(DisableProcessing, "Kudasai");
			REGISTERFUNC(DisableConsequence, "Kudasai");
			REGISTERFUNC(DisableConsequence, "Kudasai");
			REGISTERFUNC(IsConsequenceDisabled, "Kudasai");
			REGISTERFUNC(RemoveArmorByKeyword, "Kudasai");
			REGISTERFUNC(CreateFuture, "Kudasai");
			REGISTERFUNC(SortByDistance, "Kudasai");
			REGISTERFUNC(SortByDistanceRef, "Kudasai");
		}
	}

	namespace Struggle
	{
		std::vector<std::string> LookupStruggleAnimations(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions);
		std::vector<std::string> LookupBreakfreeAnimations(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions);
		std::vector<std::string> LookupKnockoutAnimations(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions);
		void SetPositions(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions);
		void ClearPositions(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, std::vector<RE::Actor*> positions);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(LookupStruggleAnimations, "KudasaiStruggle");
			REGISTERFUNC(LookupBreakfreeAnimations, "KudasaiStruggle");
			REGISTERFUNC(LookupKnockoutAnimations, "KudasaiStruggle");
			REGISTERFUNC(SetPositions, "KudasaiStruggle");
			REGISTERFUNC(ClearPositions, "KudasaiStruggle");
		}
	}

	namespace Interface
	{
		bool OpenQTEMenu(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, int32_t difficulty, RE::TESForm* callback);
		void CloseQTEMenu(RE::StaticFunctionTag*);

		inline void Register(VM* a_vm)
		{
			REGISTERFUNC(OpenQTEMenu, "Kudasai");
			REGISTERFUNC(CloseQTEMenu, "Kudasai");
		}
	}

	inline bool RegisterFuncs(VM* a_vm)
	{
		Defeat::Register(a_vm);
		ObjectRef::Register(a_vm);
		Actor::Register(a_vm);
		Config::Register(a_vm);
		Utility::Register(a_vm);
		Struggle::Register(a_vm);
		Interface::Register(a_vm);

		logger::info("Registered Functions");
		return true;
	}
}  // namespace Papyrus
