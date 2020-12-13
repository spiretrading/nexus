#ifndef NEXUS_MARKET_DATA_SERVICE_HPP
#define NEXUS_MARKET_DATA_SERVICE_HPP
#include <string>

namespace Nexus::MarketDataService {
  class ApplicationMarketDataClient;
  template<typename D> class AsyncHistoricalDataStore;
  template<typename D> class CachedHistoricalDataStore;
  template<typename C> class ClientHistoricalDataStore;
  template<typename D> class DataStoreMarketDataClient;
  class DistributedMarketDataClient;
  class EntitlementDatabase;
  struct EntitlementKey;
  class EntitlementSet;
  struct HistoricalDataStore;
  class HistoricalDataStoreBox;
  class HistoricalDataStoreException;
  template<typename V, typename D> class HistoricalDataStoreQueryWrapper;
  class LocalHistoricalDataStore;
  template<typename B> class MarketDataClient;
  class MarketDataClientBox;
  template<typename O, typename S, typename P, typename H>
    class MarketDataFeedClient;
  class MarketDataFeedClientBox;
  template<typename C, typename R> class MarketDataFeedServlet;
  template<typename T> struct MarketDataQueryType;
  class MarketDataRegistry;
  template<typename C, typename R, typename D, typename A>
    class MarketDataRegistryServlet;
  class MarketDataRegistrySession;
  template<typename C, typename M, typename A> class MarketDataRelayServlet;
  class MarketEntry;
  class SecurityEntry;
  struct SecuritySnapshot;
  template<typename D> class SessionCachedHistoricalDataStore;
  template<typename C> class SqlHistoricalDataStore;

  /** Standard name for the market data feed service. */
  inline const auto FEED_SERVICE_NAME = std::string("market_data_feed_service");

  /** Standard name for the market data registry service. */
  inline const auto REGISTRY_SERVICE_NAME =
    std::string("market_data_registry_service");

  /** Standard name for the market data relay service. */
  inline const auto RELAY_SERVICE_NAME =
    std::string("market_data_relay_service");
}

#endif
