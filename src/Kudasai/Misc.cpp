#include "Kudasai/Misc.h"

#include "SKSE/Impl/WinAPI.h"
#include <random>

namespace Kudasai
{
	void ConsolePrint(const char* msg)
	{
		const auto console = RE::ConsoleLog::GetSingleton();
		if (console) {
			console->Print(msg);
		}
	}

	void WinMsgFATAL(const char* a_msg, const char* a_cpt)
	{
		if (SKSE::WinAPI::MessageBox(nullptr, a_msg, a_cpt, 0x00000004) == 6)
			std::_Exit(EXIT_FAILURE);
	}

	int randomint(int a_min, int a_max)
	{
		std::random_device rd;
		std::uniform_int_distribution<int> dist(a_min, a_max);
		std::mt19937 mt(rd());

		return dist(mt);
	}

	float randomfloat(float a_min, float a_max)
	{
		std::random_device rd;
		std::uniform_real_distribution<float> dist(a_min, a_max);
		std::mt19937 mt(rd());

		return dist(mt);
	}

	float getavpercent(RE::Actor* a_actor, RE::ActorValue a_av)
	{
		float totalAV = a_actor->GetPermanentActorValue(a_av) + a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, a_av);
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
		using func_t = decltype(&SetVehicle);
		REL::Relocation<func_t> func{ REL::ID(36879) };
		return func(actor, vehicle);
	}

	void SetPlayerAIDriven(bool aidriven)
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		RE::VMStackID stack = 0;

		using func_t = void(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, bool);
		REL::Relocation<func_t> func{ REL::ID(54960) };
		return func(vm, stack, aidriven);
	}

	RE::TESObjectREFR* PlaceAtMe(RE::TESObjectREFR* where, RE::TESForm* what, std::uint32_t count, bool forcepersist, bool initiallydisabled)
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		RE::VMStackID stack = 0;

		using func_t = RE::TESObjectREFR*(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::TESObjectREFR*, RE::TESForm*, std::uint32_t, bool, bool);
		REL::Relocation<func_t> func{ REL::ID(55672) };
		return func(vm, stack, where, what, count, forcepersist, initiallydisabled);
	};

}  // namespace Kudasai
