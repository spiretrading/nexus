#ifndef NEXUS_BACKTESTERENVIRONMENT_HPP
#define NEXUS_BACKTESTERENVIRONMENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/Backtester/BacktesterMarketDataService.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"

namespace Nexus {
  class BacktesterEnvironment : private boost::noncopyable {
    public:
      BacktesterEnvironment(boost::posix_time::ptime startTime,
        Beam::RefType<VirtualServiceClients> serviceClients);

      BacktesterEnvironment(boost::posix_time::ptime startTime,
        boost::posix_time::ptime endTime,
        Beam::RefType<VirtualServiceClients> serviceClients);

      ~BacktesterEnvironment();

      const BacktesterEventHandler& GetEventHandler() const;

      BacktesterEventHandler& GetEventHandler();

      BacktesterMarketDataService& GetMarketDataService();

      const BacktesterMarketDataService& GetMarketDataService() const;

      void Open();

      void Close();

    private:
      VirtualServiceClients* m_serviceClients;
      BacktesterEventHandler m_eventHandler;
      BacktesterMarketDataService m_marketDataService;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline BacktesterEnvironment::BacktesterEnvironment(
      boost::posix_time::ptime startTime,
      Beam::RefType<VirtualServiceClients> serviceClients)
      : BacktesterEnvironment{startTime, boost::posix_time::pos_infin,
          Beam::Ref(serviceClients)} {}

  inline BacktesterEnvironment::BacktesterEnvironment(
      boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
      Beam::RefType<VirtualServiceClients> serviceClients)
      : m_serviceClients{serviceClients.Get()},
        m_eventHandler{startTime, endTime},
        m_marketDataService{Beam::Ref(m_eventHandler),
          Beam::Ref(m_serviceClients->GetMarketDataClient())} {}

  inline BacktesterEnvironment::~BacktesterEnvironment() {
    Close();
  }

  inline const BacktesterEventHandler&
      BacktesterEnvironment::GetEventHandler() const {
    return m_eventHandler;
  }

  inline BacktesterEventHandler& BacktesterEnvironment::GetEventHandler() {
    return m_eventHandler;
  }

  inline BacktesterMarketDataService&
      BacktesterEnvironment::GetMarketDataService() {
    return m_marketDataService;
  }

  inline const BacktesterMarketDataService&
      BacktesterEnvironment::GetMarketDataService() const {
    return m_marketDataService;
  }

  inline void BacktesterEnvironment::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_serviceClients->Open();
      m_eventHandler.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void BacktesterEnvironment::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void BacktesterEnvironment::Shutdown() {
    m_eventHandler.Close();
    m_openState.SetClosed();
  }
}

#endif
