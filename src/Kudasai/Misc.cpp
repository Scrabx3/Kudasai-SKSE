#include "Kudasai/Misc.h"

#include "Papyrus/Settings.h"

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

	bool IsLight()
	{
		static const auto result = RE::TESDataHandler::GetSingleton()->LookupForm(Kudasai::QuestAssault, ESPNAME) == nullptr;
		return result;
	}

	bool IsHunter(RE::Actor* a_actor)
	{
		static const auto hunterpride = RE::TESDataHandler::GetSingleton()->LookupForm<RE::EffectSetting>(MgEffHunterPride, ESPNAME);
		return a_actor->HasMagicEffect(hunterpride);
	}

	float getavpercent(RE::Actor* a_actor, RE::ActorValue a_av)
	{
		float tempAV = a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, a_av);
		float totalAV = a_actor->GetPermanentActorValue(a_av) + tempAV;
		float currentAV = a_actor->GetActorValue(a_av);
		return totalAV / currentAV;
	}

	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::Actor* a_actor, uint32_t a_ignoredmasks)
	{
		std::vector<RE::TESObjectARMO*> sol;
		auto inventory = a_actor->GetInventory();
		for (const auto& [form, data] : inventory) {
			if (!data.second->IsWorn() || !form->IsArmor() || !form->GetPlayable() || form->GetName()[0] == '\0') {
				continue;
			}
			const auto item = data.second.get()->GetObject()->As<RE::TESObjectARMO>();
			if (!a_ignoredmasks) {
				const auto slots = static_cast<uint32_t>(item->GetSlotMask());
				// sort out items which have no enabled slots (dont throw out if at least 1 slot matches)
				if ((slots & a_ignoredmasks) == 0)
					continue;
			}
			sol.push_back(item);
		}
		return sol;
	}

	std::vector<RE::Actor*> GetFollowers()
	{
		std::vector<RE::Actor*> ret;
		const auto processLists = RE::ProcessLists::GetSingleton();
		for (auto& actorHandle : processLists->highActorHandles) {
			if (auto actor = actorHandle.get(); actor && actor->IsPlayerTeammate()) {
				ret.push_back(actor.get());
			}
		}
		return ret;
	}

	void SetVehicle(RE::Actor* actor, RE::TESObjectREFR* vehicle)
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		RE::VMStackID stack = 0;

		using func_t = void(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::Actor*, RE::TESObjectREFR*);
		REL::Relocation<func_t> func{ RELID(53940, 54764) };
		return func(vm, stack, actor, vehicle);
	}

	void SetPlayerAIDriven(bool aidriven)
	{
		auto pl = RE::PlayerCharacter::GetSingleton();

		using func_t = void(RE::PlayerCharacter*, bool);
		REL::Relocation<func_t> func{ RELID(39507, 40586) };
		return func(pl, aidriven);
	}

	RE::TESObjectREFR* PlaceAtMe(RE::TESObjectREFR* where, RE::TESForm* what, std::uint32_t count, bool forcepersist, bool initiallydisabled)
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		RE::VMStackID stack = 0;

		using func_t = RE::TESObjectREFR*(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::TESObjectREFR*, RE::TESForm*, std::uint32_t, bool, bool);
		REL::Relocation<func_t> func{ RELID(55672, 56203) };
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
		REL::Relocation<func_t> func{ RELID(55712, 56243) };
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

	// c @ Fenix31415
	// Probably someone will find it useful.Didn't find it in clib. to call it :
	// 	auto idle = RE::TESForm::LookupByID<RE::TESIdleForm>(0x6440c);
	// PlayIdle(attacker->currentProcess, attacker, RE::DEFAULT_OBJECT::kActionIdle, idle, true, false, victim);
	bool PlayIdle(RE::AIProcess* proc, RE::Actor* attacker, RE::DEFAULT_OBJECT smth, RE::TESIdleForm* idle, bool a5, bool a6, RE::TESObjectREFR* target)
	{
		using func_t = decltype(&PlayIdle);
		REL::Relocation<func_t> func{ RELID(38290, 38290) };  // TODO: AE RELID
		return func(proc, attacker, smth, idle, a5, a6, target);
	}

	void RemoveFromFaction(RE::Actor* subject, RE::TESFaction* faction)
	{
		using func_t = decltype(&RemoveFromFaction);
		REL::Relocation<func_t> func{ RELID(36680, 36680) };  // TODO: AE RELID
		return func(subject, faction);
	}

}  // namespace Kudasai
