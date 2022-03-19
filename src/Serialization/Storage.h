#pragma once

namespace Serialize
{
	class Storage
	{
	public:
		[[nodiscard]] static Storage* GetSingleton()
		{
			static Storage singleton;
			return &singleton;
		}

		// actors currently listed as defeated
		std::set<RE::FormID> defeats;
		std::set<RE::FormID> pacifies;

	private:
		Storage() = default;
		~Storage() = default;
	};	// class Storage

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
				}
			}
		}
	}

	inline void SaveCallback(SKSE::SerializationInterface* a_intfc)
	{
		auto storage = Storage::GetSingleton();
		// Defeated Actors
		if (!a_intfc->OpenRecord('dfts', 1))
			logger::error("Failed to open record for dfts");
		else
			SaveSet<uint32_t>(a_intfc, storage->defeats);
		// Pacified Actors
		if (!a_intfc->OpenRecord('pfcy', 1))
			logger::error("Failed to open record for pfcy");
		else
			SaveSet<uint32_t>(a_intfc, storage->pacifies);
	}

	template <typename T>
	inline void LoadSet(SKSE::SerializationInterface* a_intfc, std::set<T> a_set)
	{
		size_t size;
		if (!a_intfc->ReadRecordData(size)) {
			logger::error("Failed to read record data size");
			return;
		}
		for (size_t i = 0; i < size; ++i) {
			uint32_t formID;
			if (!a_intfc->ReadRecordData(formID)) {
				logger::error("Failed to read record for ID = {}", formID);
				continue;
			}
			uint32_t newFormID;
			if (!a_intfc->ResolveFormID(formID, newFormID)) {
				logger::error("Failed to resolve ID for old ID = {}", formID);
				continue;
			}
			a_set.insert(newFormID);
			logger::info("Added Actor = {} from cosave", newFormID);
		}
	}

	inline void LoadCallback(SKSE::SerializationInterface* a_intfc)
	{
		const auto storage = Storage::GetSingleton();

		uint32_t type;
		uint32_t version;
		uint32_t length;
		while (a_intfc->GetNextRecordInfo(type, version, length)) {
			switch (type) {
			case 'dfts':
				logger::info("Loading from Save = dfts");
				LoadSet<uint32_t>(a_intfc, storage->defeats);
				break;
			case 'pfcy':
				logger::info("Loading from Save = pfcy");
				LoadSet<uint32_t>(a_intfc, storage->pacifies);
				break;
			default:
				logger::error("Unrecognized signature type");
				break;
			}
		}
	}

	inline void RevertCallback(SKSE::SerializationInterface* a_intfc)
	{
		const auto storage = Storage::GetSingleton();
		storage->defeats.clear();
		storage->pacifies.clear();
		LoadCallback(a_intfc);
	}

}  // namespace Serialize