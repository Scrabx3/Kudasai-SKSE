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
		[[nodiscard]] const bool isexcludedactor(RE::Actor* subject);	 // if this actor is to be ignored by the mod
		[[nodiscard]] const bool isstripprotec(RE::TESObjectARMO* armor);	 // if this item is protected from destruction

		[[nodiscard]] static const RE::TESForm* const getpillar();  // object for scene starting
		[[nodiscard]] static const bool isactorinterested(RE::Actor* primum, RE::Actor* secundum);  // if the secundum is (sexually) interested in primum
		[[nodiscard]] static const bool isvalidcreature(RE::Actor* subject);	 // is the creature allowed to assault another

		static void createassault(RE::Actor* primum, std::vector<RE::Actor*> secundi);	 //dispatch papyrus call to create an assault (without validation)

	private:
		using KeyPair = std::pair<uint32_t, std::string_view>;
		KeyPair createkeypair(uint32_t a_id);

		std::set<KeyPair> excludedarmor;
		std::set<KeyPair> excludedactor;


		Configuration() = default;
		~Configuration() = default;
	};	// class Interface

} // namespace Papyrus