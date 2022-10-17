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
			if (a_ignoredmasks) {
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
		using func_t = void(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::Actor*, RE::TESObjectREFR*);
		REL::Relocation<func_t> func{ RELID(53940, 54764) };
		return func(nullptr, 0, actor, vehicle);
	}

	RE::TESObjectREFR* PlaceAtMe(RE::TESObjectREFR* where, RE::TESForm* what, std::uint32_t count, bool forcepersist, bool initiallydisabled)
	{
		using func_t = RE::TESObjectREFR*(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::TESObjectREFR*, RE::TESForm*, std::uint32_t, bool, bool);
		REL::Relocation<func_t> func{ RELID(55672, 56203) };
		return func(nullptr, 0, where, what, count, forcepersist, initiallydisabled);
	};

	void SetRestrained(RE::Actor* subject, bool restrained)
	{
		if (subject->IsPlayerRef()) {
			RE::PlayerCharacter::GetSingleton()->SetAIDriven(restrained);
		} else {
			subject->actorState1.lifeState = restrained ? RE::ACTOR_LIFE_STATE::kRestrained : RE::ACTOR_LIFE_STATE::kAlive;
		}
	}

	void StopTranslating(RE::Actor* subject)
	{
		using func_t = void(RE::BSScript::Internal::VirtualMachine*, RE::VMStackID, RE::Actor*);
		REL::Relocation<func_t> func{ RELID(55712, 56243) };
		return func(nullptr, 0, subject);
	}

	void AddKeyword(RE::Actor* subject, RE::BGSKeyword* keyword, bool add)
	{
		auto ref = subject->GetObjectReference();
		if (!ref)
			return;

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

	RE::TESActorBase* GetLeveledActorBase(RE::Actor* a_actor)
	{
		const auto base = a_actor->GetTemplateActorBase();
		return base ? base : a_actor->GetActorBase();
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
		REL::Relocation<func_t> func{ RELID(36680, 37674) };
		return func(subject, faction);
	}

}  // namespace Kudasai
