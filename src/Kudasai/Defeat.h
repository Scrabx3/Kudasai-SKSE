#pragma once

namespace Kudasai::Defeat
{
	/**
     * @brief Defeat this Actor, force them into Bleedout and pacify them
     */
	void defeat(RE::Actor* subject);

	/**
     * @brief rescue this Actor from defeat
     * 
     * @param undo_pacify Should pacification also be undone
     */
	void rescue(RE::Actor* subject, const bool undo_pacify);

	/**
     * @brief Pacify this Actor, stopping them from entering combat
     */
	void pacify(RE::Actor* subject);

	/**
     * @brief remove pacification status from this actor
     */
	void undopacify(RE::Actor* subject);

	bool isdefeated(RE::Actor* subject);
	bool ispacified(RE::Actor* subject);

	// Internal
	void setdamageimmune(RE::Actor* subject, bool immune);
	bool getdamageimmune(RE::Actor* subject);

}	// namespace Defeat
