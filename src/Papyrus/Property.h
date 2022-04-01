#pragma once

namespace Papyrus
{
	using ObjectPtr = RE::BSTSmartPointer<RE::BSScript::Object>;
	inline ObjectPtr CreateObjectPtr(RE::TESForm* form, const char* classname)
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto policy = vm->GetObjectHandlePolicy();
		auto handle = policy->GetHandleForObject(form->GetFormType(), form);
		ObjectPtr object = nullptr;
		if (!vm->FindBoundObject(handle, classname, object))
			logger::critical("Settings object not found");
		return object;
	}

	template <class T>
	inline T GetProperty(ObjectPtr obj, RE::BSFixedString property)
	{
		auto var = obj->GetProperty(property);
		return RE::BSScript::UnpackValue<T>(var);
	}

	template <class T>
	inline void SetProperty(ObjectPtr obj, RE::BSFixedString property, T val)
	{
		auto var = obj->GetProperty(property);
		RE::BSScript::PackValue(var, val);
	}
}  // namespace Papyrus
