#ifndef SPIRE_SECURITY_DECK_HPP
#define SPIRE_SECURITY_DECK_HPP
#include <deque>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDeque.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Security.hpp"

namespace Spire {

  /**
   * Stores a deck of securities that a user can cycle through from the top or
   * bottom.
   */
  class SecurityDeck {
    public:

      /** Constructs an empty SecurityDeck. */
      SecurityDeck() = default;

      /** Returns the top of the deck. */
      boost::optional<Nexus::Security> get_top() const;

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
      friend struct Beam::DataShuttle;
      std::deque<Nexus::Security> m_deck;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void SecurityDeck::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("deck", m_deck);
  }
}

#endif
