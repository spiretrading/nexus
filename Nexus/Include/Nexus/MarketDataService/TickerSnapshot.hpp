#ifndef NEXUS_MARKET_DATA_TICKER_SNAPSHOT_HPP
#define NEXUS_MARKET_DATA_TICKER_SNAPSHOT_HPP
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/MarketDataService/TickerQuery.hpp"

namespace Nexus {

  /** Stores a market data snapshot of a Ticker. */
  struct TickerSnapshot {

    /** The Ticker represented. */
    Ticker m_ticker;

    /** The most recent BboQuote. */
    SequencedBboQuote m_bbo_quote;

    /** The most recent TimeAndSale. */
    SequencedTimeAndSale m_time_and_sale;

    /** The list of BookQuotes that are asks. */
    std::vector<SequencedBookQuote> m_asks;

    /** The list of BookQuotes that are bids. */
    std::vector<SequencedBookQuote> m_bids;

    /** Constructs an empty TickerSnapshot. */
    TickerSnapshot() = default;

    /**
     * Constructs a TickerSnapshot.
     * @param ticker The Ticker represented.
     */
    explicit TickerSnapshot(Ticker ticker) noexcept;

    bool operator ==(const TickerSnapshot&) const = default;
  };

  inline TickerSnapshot::TickerSnapshot(Ticker ticker) noexcept
    : m_ticker(std::move(ticker)) {}
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::TickerSnapshot> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::TickerSnapshot& value,
        unsigned int version) const {
      shuttle.shuttle("ticker", value.m_ticker);
      shuttle.shuttle("bbo_quote", value.m_bbo_quote);
      shuttle.shuttle("time_and_sale", value.m_time_and_sale);
      shuttle.shuttle("asks", value.m_asks);
      shuttle.shuttle("bids", value.m_bids);
    }
  };
}

#endif
