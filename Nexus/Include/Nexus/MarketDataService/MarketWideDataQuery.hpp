#ifndef NEXUS_MARKETDATAMARKETWIDEDATAQUERY_HPP
#define NEXUS_MARKETDATAMARKETWIDEDATAQUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus {
  typedef Beam::Queries::SequencedValue<OrderImbalance> SequencedOrderImbalance;
  typedef Beam::Queries::IndexedValue<OrderImbalance, MarketCode>
    MarketOrderImbalance;
  typedef Beam::Queries::SequencedValue<MarketOrderImbalance>
    SequencedMarketOrderImbalance;

namespace MarketDataService {

  //! Defines the type of query used to receive a market data for a market.
  typedef Beam::Queries::BasicQuery<MarketCode> MarketWideDataQuery;

  //! Builds a MarketWideDataQuery for real time data with a snapshot
  //! containing the most recent value.
  /*!
    \param market The market to query.
  */
  inline MarketWideDataQuery QueryRealTimeWithSnapshot(
      const MarketCode& market) {
    MarketWideDataQuery query;
    query.SetIndex(market);
    query.SetRange(Beam::Queries::Range::Total());
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Type::TAIL, 1);
    query.SetInterruptionPolicy(
      Beam::Queries::InterruptionPolicy::IGNORE_CONTINUE);
    return query;
  }
}
}

#endif
