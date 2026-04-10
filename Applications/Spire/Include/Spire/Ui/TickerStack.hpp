#ifndef SPIRE_TICKER_STACK_HPP
#define SPIRE_TICKER_STACK_HPP
#include <deque>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDeque.hpp>
#include "Nexus/Definitions/Ticker.hpp"

namespace Spire {

  /** Stores a stack like cycle of ticker's viewed by a user. */
  class TickerStack {
    public:

      /** Constructs an empty stack. */
      TickerStack() = default;

      /** Pushes a ticker onto the stack. */
      void push(const Nexus::Ticker& ticker);

      /**
       * Pushes a ticker onto the stack and yields the top of the stack.
       * @param ticker The ticker to push.
       * @return The top of the stack.
       */
      Nexus::Ticker push_front(const Nexus::Ticker& ticker);

      /**
       * Pushes a ticker onto the stack and pops out the bottom of the stack.
       * @param ticker The ticker to push.
       * @return The bottom of the stack.
       */
      Nexus::Ticker push_back(const Nexus::Ticker& ticker);

    private:
      friend struct Beam::DataShuttle;
      std::deque<Nexus::Ticker> m_tickers;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TickerStack::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("tickers", m_tickers);
  }
}

#endif
