#pragma once

namespace Papyrus
{
	using ObjectPtr = RE::BSTSmartPointer<RE::BSScript::Object>;
	inline ObjectPtr CreateObjectPtr(const RE::TESForm* form, const char* classname)
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto policy = vm->GetObjectHandlePolicy();
		auto handle = policy->GetHandleForObject(form->GetFormType(), form);
		ObjectPtr ret = nullptr;
		if (!vm->FindBoundObject(handle, classname, ret))
			logger::critical("Settings object not found");
		return ret;
	}

	template <class T>
	inline T GetProperty(const ObjectPtr& a_scriptobject, const RE::BSFixedString& a_property)
	{
		auto var = a_scriptobject->GetProperty(a_property);
		return RE::BSScript::UnpackValue<T>(var);
	}

	template <class T>
	inline void SetProperty(const ObjectPtr& a_scriptobject, const RE::BSFixedString& a_property, T a_value)
	{
		auto var = a_scriptobject->GetProperty(a_property);
		RE::BSScript::PackValue(var, a_value);
	}

}  // namespace Papyrus
