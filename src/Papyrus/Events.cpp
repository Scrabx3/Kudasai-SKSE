#include "Papyrus/Events.h"

#include "Serialization/EventManager.h"

namespace Papyrus
{
	void RegisterForActorDefeated(RE::StaticFunctionTag*, const RE::TESForm* form)
	{
		Serialization::EventManager::GetSingleton()->_actordefeated.Register(form);
	}
	void UnregisterForActorDefeated(RE::StaticFunctionTag*, const RE::TESForm* form)
	{
		Serialization::EventManager::GetSingleton()->_actordefeated.Unregister(form);
	}
	void RegisterForActorDefeated_Alias(RE::StaticFunctionTag*, RE::BGSBaseAlias* alias)
	{
		Serialization::EventManager::GetSingleton()->_actordefeated.Register(alias);
	}
	void UnregisterForActorDefeated_Alias(RE::StaticFunctionTag*, RE::BGSBaseAlias* alias)
	{
		Serialization::EventManager::GetSingleton()->_actordefeated.Unregister(alias);
	}
	void RegisterForActorDefeated_MgEff(RE::StaticFunctionTag*, RE::ActiveEffect* activeeffect)
	{
		Serialization::EventManager::GetSingleton()->_actordefeated.Register(activeeffect);
	}
	void UnregisterForActorDefeated_MgEff(RE::StaticFunctionTag*, RE::ActiveEffect* activeeffect)
	{
		Serialization::EventManager::GetSingleton()->_actordefeated.Unregister(activeeffect);
	}

	void RegisterForActorRescued(RE::StaticFunctionTag*, const RE::TESForm* form)
	{
		Serialization::EventManager::GetSingleton()->_actorrescued.Register(form);
	}
	void UnregisterForActorRescued(RE::StaticFunctionTag*, const RE::TESForm* form)
	{
		Serialization::EventManager::GetSingleton()->_actorrescued.Unregister(form);
	}
	void RegisterForActorRescued_Alias(RE::StaticFunctionTag*, RE::BGSBaseAlias* alias)
	{
		Serialization::EventManager::GetSingleton()->_actorrescued.Register(alias);
	}
	void UnregisterForActorRescued_Alias(RE::StaticFunctionTag*, RE::BGSBaseAlias* alias)
	{
		Serialization::EventManager::GetSingleton()->_actorrescued.Unregister(alias);
	}
	void RegisterForActorRescued_MgEff(RE::StaticFunctionTag*, RE::ActiveEffect* activeeffect)
	{
		Serialization::EventManager::GetSingleton()->_actorrescued.Register(activeeffect);
	}
	void UnregisterForActorRescued_MgEff(RE::StaticFunctionTag*, RE::ActiveEffect* activeeffect)
	{
		Serialization::EventManager::GetSingleton()->_actorrescued.Unregister(activeeffect);
	}

}  // namespace Papyrus
