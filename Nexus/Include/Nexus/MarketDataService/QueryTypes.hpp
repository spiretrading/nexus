#ifndef NEXUS_MARKET_DATA_QUERY_TYPES_HPP
#define NEXUS_MARKET_DATA_QUERY_TYPES_HPP
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

namespace Nexus::MarketDataService {

  /**
   * Type trait that specifies what type of query to use for a specified type of
   * market data.
   * @param <T> The type of market data to query.
   */
  template<typename T>
  struct MarketDataQueryType {};

  template<typename T>
  using GetMarketDataQueryType = typename MarketDataQueryType<T>::type;

  template<>
  struct MarketDataQueryType<OrderImbalance> {
    using type = MarketWideDataQuery;
  };

  template<>
  struct MarketDataQueryType<TimeAndSale> {
    using type = SecurityMarketDataQuery;
  };

  template<>
  struct MarketDataQueryType<BboQuote> {
    using type = SecurityMarketDataQuery;
  };

  template<>
  struct MarketDataQueryType<BookQuote> {
    using type = SecurityMarketDataQuery;
  };

  template<>
  struct MarketDataQueryType<MarketQuote> {
    using type = SecurityMarketDataQuery;
  };

  template<typename T>
  struct MarketDataQueryType<Beam::Queries::SequencedValue<T>> {
    using type = GetMarketDataQueryType<T>;
  };
}

#endif
