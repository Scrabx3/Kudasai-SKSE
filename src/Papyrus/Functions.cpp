#include "Papyrus/Functions.h"

#include "Papyrus/Settings.h"
#include "Kudasai/Defeat.h"
namespace Papyrus
{
	// Kudasai::Defeat

	void DefeatActor(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		Kudasai::Defeat::defeat(subject);
	}

	void RescueActor(RE::StaticFunctionTag*, RE::Actor* subject, bool undopacify)
	{
		Kudasai::Defeat::rescue(subject, undopacify);
	}

	void PacifyActor(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		Kudasai::Defeat::pacify(subject);
	}

	void UndoPacify(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		Kudasai::Defeat::undopacify(subject);
	}

	bool IsDefeated(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		return Kudasai::Defeat::isdefeated(subject);
	}

	bool IsPacified(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		return Kudasai::Defeat::ispacified(subject);
	}

	// Actor

	void SetLinkedRef(RE::StaticFunctionTag*, RE::TESObjectREFR* object, RE::TESObjectREFR* target, RE::BGSKeyword* keyword)
	{
		object->extraList.SetLinkedRef(target, keyword);
	}

	// Config

	bool ValidCreature(RE::StaticFunctionTag*, RE::Actor* subject)
	{
		return Configuration::isvalidcreature(subject);
	}

	bool IsInterrested(RE::StaticFunctionTag*, RE::Actor* primus, std::vector<RE::Actor*> secundi)
	{
		return Configuration::isinterested(primus, secundi);
	}

	// Utility

	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::StaticFunctionTag*, RE::Actor* subject)
	{
    return Kudasai::GetWornArmor(subject);
  }

	// Internal
	
	void SetDamageImmune(RE::StaticFunctionTag*, RE::Actor* subject, bool immune)
	{
		Kudasai::Defeat::setdamageimmune(subject, immune);
	}

}  // namespace Papyrus
