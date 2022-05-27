#include "Serialization/Serialize.h"

#include "Serialization/EventManager.h"

namespace Serialization
{
	void Serialize::SaveCallback(SKSE::SerializationInterface* a_intfc)
	{
		EventManager::GetSingleton()->Save(a_intfc, _Version);
		const auto Srl = GetSingleton();

		if (!a_intfc->OpenRecord(_Defeated, _Version))
			logger::error("Failed to open record <Defeated>"sv);
		else
			SaveSet(a_intfc, Srl->Defeated);
		if (!a_intfc->OpenRecord(_Pacified, _Version))
			logger::error("Failed to open record <Pacified>"sv);
		else
			SaveSet(a_intfc, Srl->Pacified);
	}

	void Serialize::LoadCallback(SKSE::SerializationInterface* a_intfc)
	{
		const auto Srl = GetSingleton();
    
		uint32_t type;
		uint32_t version;
		uint32_t length;
		while (a_intfc->GetNextRecordInfo(type, version, length)) {
			if (version != _Version) {
				logger::warn("Invalid Version for loaded Data of Type = {}. Expected = {}; Got = {}", GetTypeName(type), _Version, version);
				continue;
			}
			switch (type) {
			case _Defeated:
				logger::info("Loading record <Defeated>"sv);
				LoadSet(a_intfc, Srl->Defeated);
				break;
			case _Pacified:
				logger::info("Loading record <Pacified>"sv);
				LoadSet(a_intfc, Srl->Pacified);
				break;
			default:
				EventManager::GetSingleton()->Load(a_intfc, type);
				break;
			}
		}

		const auto handler = RE::TESDataHandler::GetSingleton();
		const auto defeat = handler->LookupForm<RE::BGSKeyword>(0x7946FF, ESPNAME);
		const auto pacify = handler->LookupForm<RE::BGSKeyword>(0x7D1354, ESPNAME);
		ApplyKeywordSet(Srl->Defeated, defeat);
		ApplyKeywordSet(Srl->Pacified, pacify);
	}

	void Serialize::RevertCallback(SKSE::SerializationInterface* a_intfc)
	{
		EventManager::GetSingleton()->Revert(a_intfc);
		const auto Srl = GetSingleton();
		const auto handler = RE::TESDataHandler::GetSingleton();
		const auto defeat = handler->LookupForm<RE::BGSKeyword>(0x7946FF, ESPNAME);
		const auto pacify = handler->LookupForm<RE::BGSKeyword>(0x7D1354, ESPNAME);
		RemoveKeywordSet(Srl->Defeated, defeat);
		RemoveKeywordSet(Srl->Pacified, pacify);

		Srl->Defeated.clear();
		Srl->Pacified.clear();
	}

	void Serialize::FormDeleteCallback(RE::VMHandle a_handle)
	{
		EventManager::GetSingleton()->FormDelete(a_handle);
  }

	FormDeletionHandler::EventResult FormDeletionHandler::ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>*)
	{
		if (a_event && a_event->formID != 0) {
			const auto Srl = Serialize::GetSingleton();
			const auto formID = a_event->formID;

			Srl->Defeated.erase(formID);
			Srl->Pacified.erase(formID);
		}

		return EventResult::kContinue;
	}

	void LoadSet(SKSE::SerializationInterface* a_intfc, std::set<RE::FormID>& a_set)
	{
		size_t size;
		if (!a_intfc->ReadRecordData(size)) {
			logger::error("Failed to read Record Data <size>");
			return;
		}
		for (size_t i = 0; i < size; i++) {
			uint32_t formID;
			if (!a_intfc->ReadRecordData(formID)) {
        logger::error("Failed to read old Record Data");
				return;
      }
			uint32_t newFormID;
			if (!a_intfc->ResolveFormID(formID, newFormID)) {
				logger::error("Failed to resolve Form ID from old ID = {}", formID);
				return;
			}
			logger::info("Restoring {} from Cosave", newFormID);
			a_set.insert(newFormID);
		}
	}

	inline void SaveSet(SKSE::SerializationInterface* a_intfc, std::set<RE::FormID>& a_set)
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

}  // namespace Serialization
