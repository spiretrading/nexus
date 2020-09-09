#ifndef NEXUS_BACKTESTER_HPP
#define NEXUS_BACKTESTER_HPP

namespace Nexus {
  class BacktesterEnvironment;
  class BacktesterEvent;
  class BacktesterEventHandler;
  template<typename H> class BacktesterHistoricalDataStore;
  class BacktesterMarketDataClient;
  class BacktesterServiceClients;
  class BacktesterTimeClient;
  class BacktesterTimer;
  template<typename H> class CutoffHistoricalDataStore;
  template<typename I, typename T> class MarketDataEvent;
  template<typename T> class MarketDataLoadEvent;
  template<typename T> class MarketDataQueryEvent;
  class TimerBacktesterEvent;
}

#endif
