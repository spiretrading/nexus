#ifndef NEXUS_MARKET_DATA_SERVICE_HPP
#define NEXUS_MARKET_DATA_SERVICE_HPP
#include <string>

namespace Nexus::MarketDataService {
  class ApplicationMarketDataClient;
  template<typename D> class BufferedHistoricalDataStore;
  template<typename D> class CachedHistoricalDataStore;
  template<typename C> class ClientHistoricalDataStore;
  template<typename D> class DataStoreMarketDataClient;
  template<typename MarketExpressionType> class DefaultCurrencyExpression;
  class DistributedMarketDataClient;
  class EntitlementDatabase;
  struct EntitlementKey;
  class EntitlementSet;
  struct HistoricalDataStore;
  class HistoricalDataStoreException;
  template<typename V, typename D> class HistoricalDataStoreQueryWrapper;
  class LocalHistoricalDataStore;
  template<typename B> class MarketDataClient;
  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
    class MarketDataFeedClient;
  template<typename ContainerType, typename MarketDataRegistryType>
    class MarketDataFeedServlet;
  template<typename MarketDataType> struct MarketDataQueryType;
  class MarketDataRegistry;
  template<typename C, typename R, typename D, typename A>
    class MarketDataRegistryServlet;
  class MarketDataRegistrySession;
  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType> class MarketDataRelayServlet;
  class MarketEntry;
  template<typename MarketDataClientType, typename MarketExpressionType,
    typename TimeRangeExpressionType> class MarketOrderImbalanceExpression;
  class SecurityEntry;
  struct SecuritySnapshot;
  template<typename D> class SessionCachedHistoricalDataStore;
  template<typename C> class SqlHistoricalDataStore;
  class VirtualHistoricalDataStore;
  class VirtualMarketDataClient;
  class VirtualMarketDataFeedClient;
  template<typename C> class WrapperHistoricalDataStore;
  template<typename C> class WrapperMarketDataClient;
  template<typename ClientType> class WrapperMarketDataFeedClient;

  // Standard name for the market data feed service.
  inline const std::string FEED_SERVICE_NAME = "market_data_feed_service";

  // Standard name for the market data registry service.
  inline const std::string REGISTRY_SERVICE_NAME =
    "market_data_registry_service";

  // Standard name for the market data relay service.
  inline const std::string RELAY_SERVICE_NAME = "market_data_relay_service";
}

#endif
