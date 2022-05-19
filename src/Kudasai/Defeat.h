#pragma once

namespace Kudasai::Defeat
{
	void defeat(RE::Actor* subject, const bool skip_animation = false);
	void rescue(RE::Actor* subject, const bool undo_pacify, const bool skip_animation = false);
	bool isdefeated(RE::Actor* subject);

	void pacify(RE::Actor* subject);
	void undopacify(RE::Actor* subject);
	bool ispacified(RE::Actor* subject);

	// Internal
	// It is expected that the actor is defeated shortly after calling it 
	void SetDamageImmune(RE::Actor* subject);
	bool IsDamageImmune(RE::Actor* subject);

	void RescueImpl(RE::Actor* subject);
	void UndoPacifyImpl(RE::Actor* subject);

}	// namespace Defeat
