#ifndef NEXUS_MARKET_DATA_SECURITY_SNAPSHOT_HPP
#define NEXUS_MARKET_DATA_SECURITY_SNAPSHOT_HPP
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

namespace Nexus::MarketDataService {

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
    SecuritySnapshot(Security security) noexcept;

    bool operator ==(const SecuritySnapshot&) const = default;
  };

  inline SecuritySnapshot::SecuritySnapshot(Security security) noexcept
    : m_security(std::move(security)) {}
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::MarketDataService::SecuritySnapshot> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::MarketDataService::SecuritySnapshot& value,
        unsigned int version) {
      shuttle.Shuttle("security", value.m_security);
      shuttle.Shuttle("bbo_quote", value.m_bbo_quote);
      shuttle.Shuttle("time_and_sale", value.m_time_and_sale);
      shuttle.Shuttle("asks", value.m_asks);
      shuttle.Shuttle("bids", value.m_bids);
    }
  };
}

#endif
