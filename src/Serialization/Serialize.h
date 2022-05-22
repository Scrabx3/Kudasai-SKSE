#pragma once

#include "Serialization/KeywordManager.h"

namespace Serialization
{
	// Primary class to handle Serialization
	class Serialize final :
		public Singleton<Serialize>
	{
	public:
		std::set<RE::FormID> Defeated;		 // currently defeated actors
		std::set<RE::FormID> Pacified;		 // currently pacified actors

	public:
		enum : std::uint32_t
		{
			_Version = 1,

			_Defeated = 'dfts',
			_Pacified = 'pfcy',
		};

		static void SaveCallback(SKSE::SerializationInterface* a_intfc);
		static void LoadCallback(SKSE::SerializationInterface* a_intfc);
		static void RevertCallback(SKSE::SerializationInterface* a_intfc);
		static void FormDeleteCallback(RE::VMHandle a_handle);
	};	// class Serialize


	class FormDeletionHandler :
		public Singleton<FormDeletionHandler>,
		public RE::BSTEventSink<RE::TESFormDeleteEvent>
	{
		using EventResult = RE::BSEventNotifyControl;

	public:
		static void Register()
		{
			RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(GetSingleton());
			logger::info("Registered form deletion event handler");
		}

		EventResult ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>*) override;
	};	// class FormDeletionHandler


	inline void LoadSet(SKSE::SerializationInterface* a_intfc, std::set<uint32_t> a_set);

	template <typename T>
	inline void SaveSet(SKSE::SerializationInterface* a_intfc, std::set<T> a_set)
	{
		size_t size = a_set.size();
		if (!a_intfc->WriteRecordData(size)) {
			logger::error("Failed to write record data size");
		} else {
			for (auto& elem : a_set) {
				if (!a_intfc->WriteRecordData(elem)) {
					logger::error("Failed to write record data = {}", elem);
					return;
				}
			}
		}
	}

	inline std::string GetTypeName(uint32_t a_type)
	{
		constexpr auto size = sizeof(uint32_t);
		std::string ret{};
		ret.resize(size);
		const char* it = reinterpret_cast<char*>(&a_type);
		for (size_t i = 0, j = size - 2; i < size - 1; i++, j--)
			ret[j] = it[i];

		return ret;
	}

}  // namespace Serialize