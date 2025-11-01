#ifndef NEXUS_MARKET_DATA_SECURITY_MARKET_DATA_QUERY_HPP
#define NEXUS_MARKET_DATA_SECURITY_MARKET_DATA_QUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/PagedQuery.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {
  using SequencedBboQuote = Beam::SequencedValue<BboQuote>;
  using SequencedBookQuote = Beam::SequencedValue<BookQuote>;
  using SequencedTimeAndSale = Beam::SequencedValue<TimeAndSale>;
  using SecurityBboQuote = Beam::IndexedValue<BboQuote, Security>;
  using SecurityBookQuote = Beam::IndexedValue<BookQuote, Security>;
  using SecurityTimeAndSale = Beam::IndexedValue<TimeAndSale, Security>;
  using SequencedSecurityBboQuote = Beam::SequencedValue<SecurityBboQuote>;
  using SequencedSecurityBookQuote = Beam::SequencedValue<SecurityBookQuote>;
  using SequencedSecurityTimeAndSale =
    Beam::SequencedValue<SecurityTimeAndSale>;

  /** Defines the type of query used to receive a Security's market data. */
  using SecurityMarketDataQuery = Beam::BasicQuery<Security>;

  /** Defines the type of query used to load SecurityInfo objects. */
  using SecurityInfoQuery = Beam::PagedQuery<Region, Security>;

  /**
   * Returns a query to retrieve the SecurityInfo for a single security.
   * @param security The Security to query.
   * @return The SecurityInfo for the given security.
   */
  inline SecurityInfoQuery make_security_info_query(const Security& security) {
    auto query = SecurityInfoQuery();
    query.set_index(security);
    query.set_snapshot_limit(Beam::SnapshotLimit::from_head(1));
    return query;
  }
}

#endif
