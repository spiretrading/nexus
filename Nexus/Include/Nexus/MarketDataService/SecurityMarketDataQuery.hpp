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
  using SequencedBboQuote = Beam::Queries::SequencedValue<BboQuote>;
  using SequencedBookQuote = Beam::Queries::SequencedValue<BookQuote>;
  using SequencedTimeAndSale = Beam::Queries::SequencedValue<TimeAndSale>;
  using SecurityBboQuote = Beam::Queries::IndexedValue<BboQuote, Security>;
  using SecurityBookQuote = Beam::Queries::IndexedValue<BookQuote, Security>;
  using SecurityTimeAndSale =
    Beam::Queries::IndexedValue<TimeAndSale, Security>;
  using SequencedSecurityBboQuote =
    Beam::Queries::SequencedValue<SecurityBboQuote>;
  using SequencedSecurityBookQuote =
    Beam::Queries::SequencedValue<SecurityBookQuote>;
  using SequencedSecurityTimeAndSale =
    Beam::Queries::SequencedValue<SecurityTimeAndSale>;

  /** Defines the type of query used to receive a Security's market data. */
  using SecurityMarketDataQuery = Beam::Queries::BasicQuery<Security>;

  /** Defines the type of query used to load SecurityInfo objects. */
  using SecurityInfoQuery = Beam::Queries::PagedQuery<Region, Security>;

  /**
   * Returns a query to retrieve the SecurityInfo for a single security.
   * @param security The Security to query.
   * @return The SecurityInfo for the given security.
   */
  inline SecurityInfoQuery make_security_info_query(const Security& security) {
    auto query = SecurityInfoQuery();
    query.SetIndex(security);
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::FromHead(1));
    return query;
  }
}

#endif
