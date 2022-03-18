#pragma once

namespace Kudasai
{
	// Debug
	void ConsolePrint(std::string a_msg);

	void WinMsgFATAL(const char* a_msg, const char* a_cpt);

	// Utility
	template <class T>
	T randomINT(T a_min, T a_max)
	{
		std::random_device rd;
		std::uniform_int_distribution<T> dist(a_min, a_max);
		std::mt19937 mt(rd());

		return dist(mt);
	}

	template <class T>
	T randomREAL(T a_min, T a_max)
	{
		std::random_device rd;
		std::uniform_real_distribution<T> dist(a_min, a_max);
		std::mt19937 mt(rd());
		return dist(mt);
	}

	// Actor
	float getavpercent(RE::Actor* a_actor, RE::ActorValue a_val);

	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::Actor* a_actor);

	void SetVehicle(RE::Actor* actor, RE::TESObjectREFR* vehicle);

	void ResetVehicle(RE::Actor* subject);

	void SetPlayerAIDriven(bool aidriven = true);

	void SetRestrained(RE::Actor* subject, bool restrained);

	void StopTranslating(RE::Actor* subject);

	void AddKeyword(RE::Actor* subject, RE::BGSKeyword* keyword, bool add = true);

	void RemoveKeyword(RE::Actor* subject, RE::BGSKeyword* keyword);

	// ObjectReference
	RE::TESObjectREFR* PlaceAtMe(RE::TESObjectREFR* where, RE::TESForm* what, std::uint32_t count = 1, bool forcePersist = false, bool initiallyDisabled = false);

}  // namespace Kudasai

// class CallbackFunctor : public RE::BSScript::IStackCallbackFunctor
// {
// public:
// 	virtual void SetResult(RE::BSScript::Variable* a_result) override
// 	{
// 		// extract function result here
// 	}


// 	virtual bool Unk_02() override
// 	{
// 		return false;
// 	}


// 	virtual void Unk_03(void) override
// 	{}
// };


// void CallFunc()
// {
// 	auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
// 	auto args = RE::MakeFunctionArguments();
// 	RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback(new CallbackFunctor());
// 	vm->CallStaticFunction("MyClass", "HelloWorld", args, callback);
// }
