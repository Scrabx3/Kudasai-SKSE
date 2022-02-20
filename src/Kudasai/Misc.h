#pragma once

namespace Kudasai
{
	// Debug
	void ConsolePrint(const char* a_msg);

	void WinMsgFATAL(const char* a_msg, const char* a_cpt);

	// Utility
	int randomint(int a_min, int a_max);

	float randomfloat(float a_min, float a_max);

	template <class T>
	T* getform(int a_formid, const char* a_pluginname);

	// Actor
	float getavpercent(RE::Actor* a_actor, RE::ActorValue a_val);

	std::vector<RE::TESObjectARMO*> GetWornArmor(RE::Actor* a_actorr);
} // namespace Kudasai
