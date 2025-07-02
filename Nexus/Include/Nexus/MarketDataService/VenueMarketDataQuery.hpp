#ifndef NEXUS_VENUE_QUERY_HPP
#define NEXUS_VENUE_QUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus {
  using SequencedOrderImbalance = Beam::Queries::SequencedValue<OrderImbalance>;
  using VenueOrderImbalance =
    Beam::Queries::IndexedValue<OrderImbalance, Venue>;
  using SequencedVenueOrderImbalance =
    Beam::Queries::SequencedValue<VenueOrderImbalance>;

namespace MarketDataService {

  //! Defines the type of query used to receive a market data for a venue.
  using VenueQuery = Beam::Queries::BasicQuery<Venue>;
}
}

#endif
