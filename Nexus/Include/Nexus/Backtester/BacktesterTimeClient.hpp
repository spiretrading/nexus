#ifndef NEXUS_BACKTESTER_TIME_CLIENT_HPP
#define NEXUS_BACKTESTER_TIME_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"

namespace Nexus {

  /** A TimeClient used by the backtester. */
  class BacktesterTimeClient {
    public:

      /**
       * Constructs a BacktesterTimeClient.
       * @param environment The event handler this client belongs to.
       */
      explicit BacktesterTimeClient(
        Beam::Ref<BacktesterEventHandler> eventHandler);

      ~BacktesterTimeClient();

      boost::posix_time::ptime GetTime();

      void Close();

    private:
      BacktesterEventHandler* m_eventHandler;
      Beam::IO::OpenState m_openState;

      BacktesterTimeClient(const BacktesterTimeClient&) = delete;
      BacktesterTimeClient& operator =(const BacktesterTimeClient&) = delete;
  };

  inline BacktesterTimeClient::BacktesterTimeClient(
    Beam::Ref<BacktesterEventHandler> eventHandler)
    : m_eventHandler(eventHandler.Get()) {}

  inline BacktesterTimeClient::~BacktesterTimeClient() {
    Close();
  }

  inline boost::posix_time::ptime BacktesterTimeClient::GetTime() {
    return m_eventHandler->GetTime();
  }

  inline void BacktesterTimeClient::Close() {
    m_openState.Close();
  }
}

#endif
