#ifndef NEXUS_BACKTESTER_TIME_CLIENT_HPP
#define NEXUS_BACKTESTER_TIME_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include "Nexus/Backtester/BacktesterEventHandler.hpp"

namespace Nexus {

  /** A TimeClient used by the backtester. */
  class BacktesterTimeClient {
    public:

      /**
       * Constructs a BacktesterTimeClient.
       * @param event_handler The event handler this client belongs to.
       */
      explicit BacktesterTimeClient(
        Beam::Ref<BacktesterEventHandler> event_handler) noexcept;

      ~BacktesterTimeClient();

      boost::posix_time::ptime GetTime();

      void Close();

    private:
      BacktesterEventHandler* m_event_handler;
      Beam::IO::OpenState m_open_state;

      BacktesterTimeClient(const BacktesterTimeClient&) = delete;
      BacktesterTimeClient& operator =(const BacktesterTimeClient&) = delete;
  };

  inline BacktesterTimeClient::BacktesterTimeClient(
    Beam::Ref<BacktesterEventHandler> event_handler) noexcept
    : m_event_handler(event_handler.Get()) {}

  inline BacktesterTimeClient::~BacktesterTimeClient() {
    Close();
  }

  inline boost::posix_time::ptime BacktesterTimeClient::GetTime() {
    return m_event_handler->get_time();
  }

  inline void BacktesterTimeClient::Close() {
    m_open_state.Close();
  }
}

#endif
