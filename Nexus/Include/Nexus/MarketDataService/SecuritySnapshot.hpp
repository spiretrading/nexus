#ifndef NEXUS_MARKET_DATA_SECURITY_SNAPSHOT_HPP
#define NEXUS_MARKET_DATA_SECURITY_SNAPSHOT_HPP
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

namespace Nexus {

  /** Stores a market data snapshot of a Security. */
  struct SecuritySnapshot {

    /** The Security represented. */
    Security m_security;

    /** The most recent BboQuote. */
    SequencedBboQuote m_bbo_quote;

    /** The most recent TimeAndSale. */
    SequencedTimeAndSale m_time_and_sale;

    /** The list of BookQuotes that are asks. */
    std::vector<SequencedBookQuote> m_asks;

    /** The list of BookQuotes that are bids. */
    std::vector<SequencedBookQuote> m_bids;

    /** Constructs an empty SecuritySnapshot. */
    SecuritySnapshot() = default;

    /**
     * Constructs a SecuritySnapshot.
     * @param security The Security represented.
     */
    explicit SecuritySnapshot(Security security) noexcept;

    bool operator ==(const SecuritySnapshot&) const = default;
  };

  inline SecuritySnapshot::SecuritySnapshot(Security security) noexcept
    : m_security(std::move(security)) {}
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::SecuritySnapshot> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::SecuritySnapshot& value,
        unsigned int version) const {
      shuttle.shuttle("security", value.m_security);
      shuttle.shuttle("bbo_quote", value.m_bbo_quote);
      shuttle.shuttle("time_and_sale", value.m_time_and_sale);
      shuttle.shuttle("asks", value.m_asks);
      shuttle.shuttle("bids", value.m_bids);
    }
  };
}

#endif
