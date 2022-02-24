#pragma once

namespace Papyrus
{
	using ObjectPtr = RE::BSTSmartPointer<RE::BSScript::Object>;
	inline ObjectPtr GetSettingsObject()
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto form = RE::TESDataHandler::GetSingleton()->LookupForm(0x7853F1, "YKudasai.esp");  // main q
		auto policy = vm->GetObjectHandlePolicy();
		auto handle = policy->GetHandleForObject(form->GetFormType(), form);
		ObjectPtr object = nullptr;
		if (!vm->FindBoundObject(handle, "KudasaiMCM", object))
			logger::critical("Settings object not found");
		return object;
	}

	template <class T>
	inline T GetProperty(RE::BSFixedString property)
	{
		const static auto obj = GetSettingsObject();
		auto var = obj->GetProperty(property);
		return RE::BSScript::UnpackValue<T>(var);
	}

	template <class T>
	inline void SetProperty(RE::BSFixedString property, T val)
	{
		auto var = GetSettingsObject()->GetProperty(property);
		if (!var)
			return;
		RE::BSScript::PackValue(var, val);
	}

	class Configuration
	{
	public:
		[[nodiscard]] static Configuration* GetSingleton()
		{
			static Configuration singleton;
			return &singleton;
		}

		[[nodiscard]] const RE::TESForm* const getpillar() const;  // the object to use place actors for animations

		// if the secundum is (sexually) interested in primum
		[[nodiscard]] bool isactorinterested(RE::Actor* primum, RE::Actor* secundum);

		// if this actor is to be ignored by the mod
		[[nodiscard]] bool isexcludedactor(RE::Actor* subject);

		// if this item is protected from destruction
		[[nodiscard]] bool isstripprotec(RE::TESObjectARMO* armor);

		// dispatch a Papyrus Call to create an adult scene. Does NOT validate
		void createassault(RE::Actor* primum, std::vector<RE::Actor*> secundi);

	private:
		std::set<uint32_t> excludedarmor;
		std::set<uint32_t> excludedactor;

		Configuration() = default;
		~Configuration() = default;
	};	// class Interface

} // namespace Papyrus