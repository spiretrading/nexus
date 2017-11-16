#ifndef NEXUS_MARKETDATAMARKETWIDEDATAQUERY_HPP
#define NEXUS_MARKETDATAMARKETWIDEDATAQUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus {
  using SequencedOrderImbalance = Beam::Queries::SequencedValue<OrderImbalance>;
  using MarketOrderImbalance =
    Beam::Queries::IndexedValue<OrderImbalance, MarketCode>;
  using SequencedMarketOrderImbalance =
    Beam::Queries::SequencedValue<MarketOrderImbalance>;

namespace MarketDataService {

  //! Defines the type of query used to receive a market data for a market.
  using MarketWideDataQuery = Beam::Queries::BasicQuery<MarketCode>;
}
}

#endif
