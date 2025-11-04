#ifndef NEXUS_MARKET_DATA_QUERY_TYPES_HPP
#define NEXUS_MARKET_DATA_QUERY_TYPES_HPP
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

namespace Nexus {

  /**
   * Type trait that specifies what type of query to use for a specified type of
   * market data.
   * @param <T> The type of market data to query.
   */
  template<typename T>
  struct market_data_query_type {};

  template<typename T>
  using market_data_query_type_t = typename market_data_query_type<T>::type;

  template<>
  struct market_data_query_type<OrderImbalance> {
    using type = VenueMarketDataQuery;
  };

  template<>
  struct market_data_query_type<TimeAndSale> {
    using type = SecurityMarketDataQuery;
  };

  template<>
  struct market_data_query_type<BboQuote> {
    using type = SecurityMarketDataQuery;
  };

  template<>
  struct market_data_query_type<BookQuote> {
    using type = SecurityMarketDataQuery;
  };

  template<typename T>
  struct market_data_query_type<Beam::SequencedValue<T>> {
    using type = market_data_query_type_t<T>;
  };
}

#endif
