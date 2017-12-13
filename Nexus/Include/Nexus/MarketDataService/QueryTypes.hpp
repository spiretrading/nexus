#ifndef NEXUS_MARKETDATAQUERYTYPES_HPP
#define NEXUS_MARKETDATAQUERYTYPES_HPP
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \struct MarketDataQueryType
      \brief Type trait that specifies what type of query to use for a specified
             type of market data.
      \tparam MarketDataType The type of market data to query.
   */
  template<typename MarketDataType>
  struct MarketDataQueryType {};

  template<typename MarketDataType>
  using GetMarketDataQueryType = typename MarketDataQueryType<
    MarketDataType>::type;

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
}

#endif
