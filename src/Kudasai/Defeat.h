#pragma once

namespace Kudasai
{
  namespace Defeat
  {
    /**
     * @brief Defeat this Actor, force them into Bleedout, disable their Aggression & make them imune to damage
     */
    void defeatactor(RE::Actor* subject, const bool forcebleedout);

    /**
     * @brief Pull this Actor out of their defeat state. Calling this on an Actor that isnt defeated is considered undefined behavior
     */
    void restoreactor(RE::Actor* subject, const bool rescue);

    /**
     * @brief check for defeat
     * 
     * @return if this actor is defeated 
     */
    bool isdefeated(RE::Actor* subject);
  }; // namespace Defeat
  
} // namespace Kudasai
