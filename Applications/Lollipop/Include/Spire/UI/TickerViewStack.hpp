#ifndef SPIRE_TICKER_VIEW_STACK_HPP
#define SPIRE_TICKER_VIEW_STACK_HPP
#include <deque>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDeque.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Ticker.hpp"

namespace Spire {
namespace UI {

  /*! \class TickerViewStack
      \brief Stores a stack like cycle of Ticker's viewed by a user.
   */
  class TickerViewStack {
    public:

      //! Constructs a TickerViewStack.
      TickerViewStack();

      //! Pushes a Ticker onto the stack.
      void Push(const Nexus::Ticker& ticker);

      //! Pushes a Ticker onto the stack and yields the top of the stack.
      /*!
        \param ticker The Ticker to push.
        \param continuation The action to perform on the top of the stack.
      */
      void PushUp(const Nexus::Ticker& ticker,
        const std::function<void (const Nexus::Ticker&)>& continuation);

      //! Pushes a Ticker onto the stack and pops out the bottom of the stack.
      /*!
        \param ticker The Ticker to push.
        \param continuation The action to perform on the bottom of the stack.
      */
      void PushDown(const Nexus::Ticker& ticker,
        const std::function<void (const Nexus::Ticker&)>& continuation);

    private:
      friend struct Beam::DataShuttle;
      std::deque<Nexus::Ticker> m_tickers;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TickerViewStack::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("tickers", m_tickers);
  }
}
}

#endif
