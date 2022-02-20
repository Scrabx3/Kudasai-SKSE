#pragma once

namespace Papyrus
{
	namespace Integration
	{
		class Configuration
		{
		public:
			[[nodiscard]] static Configuration* GetSingleton()
			{
				static Configuration singleton;
				return &singleton;
			}

			void ReadSettings();
			struct Config	{
				bool active;  					// if the mod is currently enabled.. might want to look for a better way to communicate this zzz
				int scenario;						// 1 << Mid Combat << Post Combat << Always << Never
				bool strugglies;				// struggle minigame enabled?
				// Lethality
				int preventdeath;	 			// 1 << never << by chance << player only
				int preventdchance;	 		// 0 ~ 100 | Chance that a lethal blow will be denied (if always)
				// Exhaustion
				int exhaustchance;			// Chance for a hit to knock you down while exhausted
				int staminathresh;			// 0 ~ 100 | Amount left Stamina at which defeats become valid
				int magickathresh;			// 0 ~ 100 | Amount left Magicka at which defeats become valid
				// Exposure
				int armorthresh;  			// Number worn armor at which defeats become valid
				// Stripping
				int stripchance;  			// 0 ~ 100 | Chance for a hit to strip worn armor
				int strpchdstry;  			// 0 ~ 100 | Chance for a stripped Item to be destroyed
				bool strpdrop;		  		// drop the item that has been stripped (if not destroyed)
				bool strpnotify;	  		// notification on armor destruction
			}; // struct Config

			[[nodiscard]] const Config* const getsettings() const { return &this->settings; }
			[[nodiscard]] const RE::TESForm* const getpillar() const; // the object to use place actors for animations
			
			// if the secundum is (sexually) interested in primum
			[[nodiscard]] bool isactorinterested(RE::Actor* primum, RE::Actor* secundum);

			// if this actor is to be ignored by the mod
			[[nodiscard]] bool isexcludedactor(RE::Actor* subject);

			// if this item is protected from destruction
			[[nodiscard]] bool isstripprotec(RE::TESObjectARMO* armor);

			// dispatch a Papyrus Call to create an adult scene. Does NOT validate
			void createassault(RE::Actor* primum, std::vector<RE::Actor*> secundi);

		private:
			Config settings;
			std::set<uint32_t> excludedarmor;
			std::set<uint32_t> excludedactor;

			Configuration() = default;
			~Configuration() = default;
		}; // class Interface
	} // namespace Integration

} // namespace Papyrus