#pragma once

namespace Kudasai
{
	// Debug
	void ConsolePrint(const char* a_msg);

	void WinMsgFATAL(const char* a_msg, const char* a_cpt);

	// Utility
	int randomint(int a_min, int a_max);

	float randomfloat(float a_min, float a_max);

	// Actor
	float getavpercent(RE::Actor* a_actor, RE::ActorValue a_val);

	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::Actor* a_actorr);
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
