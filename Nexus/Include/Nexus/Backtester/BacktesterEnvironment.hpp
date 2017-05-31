#ifndef NEXUS_BACKTESTERENVIRONMENT_HPP
#define NEXUS_BACKTESTERENVIRONMENT_HPP
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/Backtester/BacktesterMarketDataService.hpp"

namespace Nexus {
  class BacktesterEnvironment : private boost::noncopyable {
    public:
      const BacktesterEventHandler& GetEventHandler() const;

      BacktesterEventHandler& GetEventHandler();

      BacktesterMarketDataService& GetMarketDataService();

      const BacktesterMarketDataService& GetMarketDataService() const;

    private:
      BacktesterEventHandler m_eventHandler;
      BacktesterMarketDataService m_marketDataService;
  };

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
}

#endif
