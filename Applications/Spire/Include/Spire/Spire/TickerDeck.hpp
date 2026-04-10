#ifndef SPIRE_TICKER_DECK_HPP
#define SPIRE_TICKER_DECK_HPP
#include <deque>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDeque.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Ticker.hpp"

namespace Spire {

  /**
   * Stores a deck of tickers that a user can cycle through from the top or
   * bottom.
   */
  class TickerDeck {
    public:

      /** Constructs an empty TickerDeck. */
      TickerDeck() = default;

      /** Returns the top of the deck. */
      boost::optional<Nexus::Ticker> get_top() const;

      /**
       * Adds a Ticker to the top of the deck indicating that it's the currently
       * selected ticker. If the ticker is already in the deck, then it's moved
       * to the top.
       */
      void add(const Nexus::Ticker& ticker);

      /**
       * Takes the ticker on the bottom of the deck and moves it to the top.
       * Returns the top of the deck. If the deck is empty then <i>none</i> is
       * returned.
       */
      boost::optional<Nexus::Ticker> rotate_bottom();

      /**
       * Takes the ticker on the top of the deck and moves it to the bottom.
       * Returns the top of the deck. If the deck is empty then <i>none</i> is
       * returned.
       */
      boost::optional<Nexus::Ticker> rotate_top();

    private:
      friend struct Beam::DataShuttle;
      std::deque<Nexus::Ticker> m_deck;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TickerDeck::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("deck", m_deck);
  }
}

#endif
