#include "Kudasai/Misc.h"

#include "SKSE/Impl/WinAPI.h"
#include <random>

namespace Kudasai
{
	void ConsolePrint(std::string msg)
	{
		const auto console = RE::ConsoleLog::GetSingleton();
		if (console) {
			console->Print(msg.c_str());
		}
	}

	void WinMsgFATAL(const char* a_msg, const char* a_cpt)
	{
		if (SKSE::WinAPI::MessageBox(nullptr, a_msg, a_cpt, 0x00000004) == 6)
			std::_Exit(EXIT_FAILURE);
	}

	float getavpercent(RE::Actor* a_actor, RE::ActorValue a_av)
	{
		float tempAV = a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, a_av);
		float totalAV = a_actor->GetPermanentActorValue(a_av) + tempAV;
		float currentAV = a_actor->GetActorValue(a_av);
		return totalAV / currentAV;
	}

	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::Actor* a_actor)
	{
		std::vector<RE::TESObjectARMO*> sol;

		auto inventory = a_actor->GetInventory();
		for (const auto& [form, data] : inventory) {
			if (!data.second->IsWorn() || !form->IsArmor()) {
				continue;
			}
			const auto item = data.second.get()->GetObject()->As<RE::TESObjectARMO>();
			sol.push_back(item);
			logger::info("<GetWornArmor> Found: {}", item->GetFormID());
		}
		return sol;
	}

	void SetVehicle(RE::Actor* actor, RE::TESObjectREFR* vehicle)
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		RE::VMStackID stack = 0;

		using func_t = void(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::Actor*, RE::TESObjectREFR*);
		REL::Relocation<func_t> func{ REL::ID(53940) };
		return func(vm, stack, actor, vehicle);
	}

	void SetPlayerAIDriven(bool aidriven)
	{
		auto pl = RE::PlayerCharacter::GetSingleton();

		using func_t = void(RE::PlayerCharacter*, bool);
		REL::Relocation<func_t> func{ REL::ID(39507) };
		return func(pl, aidriven);
	}

	RE::TESObjectREFR* PlaceAtMe(RE::TESObjectREFR* where, RE::TESForm* what, std::uint32_t count, bool forcepersist, bool initiallydisabled)
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		RE::VMStackID stack = 0;

		using func_t = RE::TESObjectREFR*(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::TESObjectREFR*, RE::TESForm*, std::uint32_t, bool, bool);
		REL::Relocation<func_t> func{ REL::ID(55672) };
		return func(vm, stack, where, what, count, forcepersist, initiallydisabled);
	};

	void SetRestrained(RE::Actor* subject, bool restrained)
	{
		if (subject->IsPlayerRef())
			SetPlayerAIDriven(restrained);
		else if (restrained)
			subject->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kRestrained;
		else
			subject->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kAlive;
	}

	void StopTranslating(RE::Actor* subject)
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		RE::VMStackID stack = 0;

		using func_t = void(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::Actor*);
		REL::Relocation<func_t> func{ REL::ID(55712) };
		return func(vm, stack, subject);
	}

	void AddKeyword(RE::Actor* subject, RE::BGSKeyword* keyword, bool add)
	{
		auto ref = subject->GetObjectReference();
		if (!ref) {
			logger::warn("BoundObject from subject = {} does not exist? Skipping Keyword removal", subject->GetFormID());
			return;
		}
		if (add)
			ref->As<RE::BGSKeywordForm>()->AddKeyword(keyword);
		else
			ref->As<RE::BGSKeywordForm>()->RemoveKeyword(keyword);
	}

	void RemoveKeyword(RE::Actor* subject, RE::BGSKeyword* keyword)
	{
		AddKeyword(subject, keyword, false);
	}

	void SheatheWeapon(RE::Actor* subject)
	{
		const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
		const auto script = factory ? factory->Create() : nullptr;
		if (script) {
			script->SetCommand("rae WeaponSheathe"sv);
			script->CompileAndRun(subject);
			delete script;
		}
	}

}  // namespace Kudasai
