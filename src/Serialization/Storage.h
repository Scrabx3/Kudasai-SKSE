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

	private:
		Storage() = default;
		~Storage() = default;
	};	// class Storage

	inline void SaveCallback(SKSE::SerializationInterface* a_intfc)
	{
		auto storage = Storage::GetSingleton();

		if (!a_intfc->OpenRecord('dfts', 1)) {
			logger::error("Failed to open record for defeated Actors");
		} else {
			size_t size = storage->defeats.size();
			if (!a_intfc->WriteRecordData(size)) {
				logger::error("Failed to write record data size for defeated actors");
			} else {
				for (auto& elem : storage->defeats) {
					if (!a_intfc->WriteRecordData(elem)) {
						logger::error("Failed to write record data for defeated actors");
						break;
					}
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
				{
					size_t size;
					if (!a_intfc->ReadRecordData(size)) {
						logger::error("Failed to load size for type = dfts");
						break;
					}
					for (size_t i = 0; i < size; ++i) {
						uint32_t elem;
						if (!a_intfc->ReadRecordData(elem)) {
							logger::error("Failed to load element for type = dfts");
							break;
						} else {
							auto form = RE::TESForm::LookupByID(elem);
							if (form != nullptr && form->Is(RE::FormType::ActorCharacter) && !form->As<RE::Actor>()->IsDead()) {
								logger::info("Added defeated Actor = {} from cosave", elem);
								storage->defeats.emplace(elem);
							} else {
								logger::info("Failed to add defeated Actor = {} from cosave, form is no longer valid", elem);
							}
						}
					}
				}
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
		LoadCallback(a_intfc);
	}

}  // namespace Serialize