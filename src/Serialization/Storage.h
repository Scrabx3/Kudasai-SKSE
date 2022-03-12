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
		std::set<uint32_t> defeats;
		std::set<uint32_t> pacifies;

	private:
		Storage() = default;
		~Storage() = default;
	};	// class Storage

	template <typename T>
	inline void SaveSet(SKSE::SerializationInterface* a_intfc, std::set<T> a_set)
	{
		size_t size = a_set.size();
		if (!a_intfc->WriteRecordData(size)) {
			logger::error("Failed to write record data size for defeated actors");
		} else {
			for (auto& elem : a_set) {
				if (!a_intfc->WriteRecordData(elem)) {
					logger::error("Failed to write record data for defeated actors");
					return;
				}
			}
		}
	}

	inline void SaveCallback(SKSE::SerializationInterface* a_intfc)
	{
		auto storage = Storage::GetSingleton();

		if (!a_intfc->OpenRecord('dfts', 1)) {
			logger::error("Failed to open record for defeated Actors");
		} else {
			SaveSet<uint32_t>(a_intfc, storage->defeats);
		}
		if (!a_intfc->OpenRecord('pfcy', 1)) {
			logger::error("Failed to open record for pacified Actors");
		} else {
			SaveSet<uint32_t>(a_intfc, storage->pacifies);
		}
	}

	template <typename T>
	inline void LoadSet(SKSE::SerializationInterface* a_intfc, std::set<T> a_set)
	{
		size_t size;
		if (!a_intfc->ReadRecordData(size)) {
			logger::error("Failed to load size for type = dfts");
			return;
		}
		for (size_t i = 0; i < size; ++i) {
			uint32_t elem;
			if (!a_intfc->ReadRecordData(elem)) {
				logger::error("Failed to load element for type = dfts");
				return;
			} else {
				auto form = RE::TESForm::LookupByID(elem);
				if (form != nullptr && form->Is(RE::FormType::ActorCharacter) && !form->As<RE::Actor>()->IsDead()) {
					logger::info("Added Actor = {} from cosave", elem);
					a_set.emplace(elem);
				} else {
					logger::info("Failed to add Actor = {} from cosave, form is no longer valid", elem);
				}
			}
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
				LoadSet<uint32_t>(a_intfc, storage->defeats);
				break;
			case 'pfcy':
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