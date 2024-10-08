#ifndef SPIRE_SECURITY_DECK_HPP
#define SPIRE_SECURITY_DECK_HPP
#include <deque>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Stores a deck of securities that a user can cycle through from the top or
   * bottom.
   */
  class SecurityDeck {
    public:

      /** Constructs an empty SecurityDeck. */
      SecurityDeck() = default;

      /**
       * Adds a Security to the top of the deck indicating that it's the
       * currently selected security. If the security is already in the deck,
       * then it's moved to the top.
       */
      void add(const Nexus::Security& security);

      /**
       * Takes the security on the bottom of the deck and moves it to the top.
       * Returns the top of the deck. If the deck is empty then <i>none</i> is
       * returned.
       */
      boost::optional<Nexus::Security> rotate_bottom();

      /**
       * Takes the security on the top of the deck and moves it to the bottom.
       * Returns the top of the deck. If the deck is empty then <i>none</i> is
       * returned.
       */
      boost::optional<Nexus::Security> rotate_top();

    private:
      std::deque<Nexus::Security> m_deck;
  };
}

#endif
