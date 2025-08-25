#ifndef NEXUS_VENUE_MARKET_QUERY_HPP
#define NEXUS_VENUE_MARKET_QUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {
  using SequencedOrderImbalance = Beam::Queries::SequencedValue<OrderImbalance>;
  using VenueOrderImbalance =
    Beam::Queries::IndexedValue<OrderImbalance, Venue>;
  using SequencedVenueOrderImbalance =
    Beam::Queries::SequencedValue<VenueOrderImbalance>;

  /** Defines the type of query used to receive a market data for a venue. */
  using VenueMarketDataQuery = Beam::Queries::BasicQuery<Venue>;
}

#endif
