#ifndef NEXUS_MARKETDATASERVICE_HPP
#define NEXUS_MARKETDATASERVICE_HPP
#include <string>

namespace Nexus {
namespace MarketDataService {
  class ApplicationMarketDataClient;
  template<typename HistoricalDataStoreType> class BufferedHistoricalDataStore;
  template<typename HistoricalDataStoreType> class CachedHistoricalDataStore;
  template<typename MarketExpressionType> class DefaultCurrencyExpression;
  class EntitlementDatabase;
  struct EntitlementKey;
  class EntitlementSet;
  struct HistoricalDataStore;
  class HistoricalDataStoreException;
  template<typename T, typename DataStoreType>
    class HistoricalDataStoreQueryWrapper;
  class LocalHistoricalDataStore;
  template<typename ServiceProtocolClientBuilderType> class MarketDataClient;
  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
    class MarketDataFeedClient;
  template<typename ContainerType, typename MarketDataRegistryType>
    class MarketDataFeedServlet;
  template<typename MarketDataType> struct MarketDataQueryType;
  class MarketDataRegistry;
  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
    class MarketDataRegistryServlet;
  class MarketDataRegistrySession;
  class MarketEntry;
  template<typename MarketDataClientType, typename MarketExpressionType,
    typename TimeRangeExpressionType> class MarketOrderImbalanceExpression;
  class MySqlHistoricalDataStore;
  template<typename MarketDataClientType> class RealTimeMarketDataPublisher;
  class SecurityEntry;
  struct SecuritySnapshot;
  template<typename HistoricalDataStoreType>
    class SessionCachedHistoricalDataStore;
  class VirtualMarketDataClient;
  class VirtualMarketDataFeedClient;
  template<typename ClientType> class WrapperMarketDataClient;
  template<typename ClientType> class WrapperMarketDataFeedClient;

  // Standard name for the market data feed service.
  static const std::string FEED_SERVICE_NAME = "market_data_feed_service";

  // Standard name for the market data registry service.
  static const std::string REGISTRY_SERVICE_NAME =
    "market_data_registry_service";

  // Standard name for the market data relay service.
  static const std::string RELAY_SERVICE_NAME = "market_data_relay_service";
}
}

#endif
