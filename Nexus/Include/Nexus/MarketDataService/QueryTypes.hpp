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
  struct MarketDataQueryType<SequencedOrderImbalance> {
    using type = MarketWideDataQuery;
  };

  template<>
  struct MarketDataQueryType<SequencedTimeAndSale> {
    using type = SecurityMarketDataQuery;
  };

  template<>
  struct MarketDataQueryType<SequencedBboQuote> {
    using type = SecurityMarketDataQuery;
  };

  template<>
  struct MarketDataQueryType<SequencedBookQuote> {
    using type = SecurityMarketDataQuery;
  };

  template<>
  struct MarketDataQueryType<SequencedMarketQuote> {
    using type = SecurityMarketDataQuery;
  };
}
}

#endif
