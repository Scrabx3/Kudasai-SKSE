#pragma once

namespace Papyrus
{


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