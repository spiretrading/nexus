#ifndef NEXUS_MARKET_DATA_TICKER_QUERY_HPP
#define NEXUS_MARKET_DATA_TICKER_QUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/PagedQuery.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Scope.hpp"
#include "Nexus/Definitions/TickerInfo.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {
  using SequencedBboQuote = Beam::SequencedValue<BboQuote>;
  using SequencedBookQuote = Beam::SequencedValue<BookQuote>;
  using SequencedTimeAndSale = Beam::SequencedValue<TimeAndSale>;
  using TickerBboQuote = Beam::IndexedValue<BboQuote, Ticker>;
  using TickerBookQuote = Beam::IndexedValue<BookQuote, Ticker>;
  using TickerTimeAndSale = Beam::IndexedValue<TimeAndSale, Ticker>;
  using SequencedTickerBboQuote = Beam::SequencedValue<TickerBboQuote>;
  using SequencedTickerBookQuote = Beam::SequencedValue<TickerBookQuote>;
  using SequencedTickerTimeAndSale = Beam::SequencedValue<TickerTimeAndSale>;

  /** Defines the type of query used to receive a Ticker's market data. */
  using TickerQuery = Beam::BasicQuery<Ticker>;

  /** Defines the type of query used to load TickerInfo objects. */
  using TickerInfoQuery = Beam::PagedQuery<Scope, Ticker>;

  /**
   * Returns a query to retrieve the TickerInfo for a single ticker.
   * @param ticker The Ticker to query.
   * @return The TickerInfo for the given ticker.
   */
  inline TickerInfoQuery make_ticker_info_query(const Ticker& ticker) {
    auto query = TickerInfoQuery();
    query.set_index(ticker);
    query.set_snapshot_limit(Beam::SnapshotLimit::from_head(1));
    return query;
  }
}

#endif
