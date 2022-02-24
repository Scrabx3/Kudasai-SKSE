
namespace Papyrus
{

	bool Configuration::isstripprotec(RE::TESObjectARMO* a_armor)
	{
		if (a_armor->HasKeyword(0x0A8668))	// daedric artifact
			return true;
		return excludedarmor.contains(a_armor->GetFormID());
	}

	bool Configuration::isexcludedactor(RE::Actor* subject)
	{
		return excludedactor.contains(subject->GetFormID());
	}

	bool Configuration::isactorinterested(RE::Actor* primum, RE::Actor* secundum)
	{
		return primum != nullptr && secundum != nullptr;
	}

	void Configuration::createassault(RE::Actor* primum, std::vector<RE::Actor*> secundi)
	{
		const auto svm = RE::SkyrimVM::GetSingleton();
		auto vm = svm ? svm->impl : nullptr;
		if (vm) {
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			auto args = RE::MakeFunctionArguments(std::move(primum), std::move(secundi));
			vm->DispatchStaticCall("KudasaiAnimation", "CreateAssault", args, callback);
			delete args;
		}
	}

}  // namespace Kuasai