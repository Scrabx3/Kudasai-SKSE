#pragma once

namespace Serialization
{
	class EventManager :
		public Singleton<EventManager>
	{
	public:
		enum : std::uint32_t
		{
			ActorDefeated = 'adtd',
			ActorRescued = 'arsd',
		};

	public:
		SKSE::RegistrationSet<const RE::Actor*> _actordefeated{ "OnActorDefeated"sv };
		SKSE::RegistrationSet<const RE::Actor*> _actorrescued{ "OnActorRescued"sv };

	public:
		void Save(SKSE::SerializationInterface* a_intfc, std::uint32_t a_version);
		void Load(SKSE::SerializationInterface* a_intfc, std::uint32_t a_type);
		void Revert(SKSE::SerializationInterface* a_intfc);
		void FormDelete(RE::VMHandle a_handle);
	};

}  // namespace Serialization
