#ifndef NEXUS_BACKTESTER_TIME_CLIENT_HPP
#define NEXUS_BACKTESTER_TIME_CLIENT_HPP
#include <boost/noncopyable.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"

namespace Nexus {

  /** A TimeClient used by the backtester. */
  class BacktesterTimeClient : private boost::noncopyable {
    public:

      /**
       * Constructs a BacktesterTimeClient.
       * @param environment The event handler this client belongs to.
       */
      BacktesterTimeClient(Beam::Ref<BacktesterEventHandler> eventHandler);

      ~BacktesterTimeClient();

      boost::posix_time::ptime GetTime();

      void Close();

    private:
      BacktesterEventHandler* m_eventHandler;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline BacktesterTimeClient::BacktesterTimeClient(
      Beam::Ref<BacktesterEventHandler> eventHandler)
      : m_eventHandler(eventHandler.Get()) {
    m_openState.SetOpen();
  }

  inline BacktesterTimeClient::~BacktesterTimeClient() {
    Close();
  }

  inline boost::posix_time::ptime BacktesterTimeClient::GetTime() {
    return m_eventHandler->GetTime();
  }

  inline void BacktesterTimeClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void BacktesterTimeClient::Shutdown() {
    m_openState.SetClosed();
  }
}

#endif
