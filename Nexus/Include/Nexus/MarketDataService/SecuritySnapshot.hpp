#ifndef NEXUS_MARKET_DATA_SECURITY_SNAPSHOT_HPP
#define NEXUS_MARKET_DATA_SECURITY_SNAPSHOT_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketDataType.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

namespace Nexus::MarketDataService {

  /* Stores a market data snapshot of a Security. */
  struct SecuritySnapshot {

    //! The Security represented.
    Security m_security;

    //! The most recent BboQuote.
    SequencedBboQuote m_bboQuote;

    //! The most recent TimeAndSale.
    SequencedTimeAndSale m_timeAndSale;

    //! The list of MarketQuotes.
    std::unordered_map<MarketCode, SequencedMarketQuote> m_marketQuotes;

    //! The list of BookQuotes that are ASKs.
    std::vector<SequencedBookQuote> m_askBook;

    //! The list of BookQuotes that are BIDs.
    std::vector<SequencedBookQuote> m_bidBook;

    //! Constructs a SecuritySnapshot.
    SecuritySnapshot() = default;

    //! Constructs a SecuritySnapshot.
    /*!
      \param security The Security represented.
    */
    SecuritySnapshot(const Security& security);
  };

  inline SecuritySnapshot::SecuritySnapshot(const Security& security)
      : m_security(security) {}
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::MarketDataService::SecuritySnapshot> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::MarketDataService::SecuritySnapshot& value,
        unsigned int version) {
      shuttle.Shuttle("security", value.m_security);
      shuttle.Shuttle("bbo_quote", value.m_bboQuote);
      shuttle.Shuttle("time_and_sale", value.m_timeAndSale);
      shuttle.Shuttle("market_quotes", value.m_marketQuotes);
      shuttle.Shuttle("ask_book", value.m_askBook);
      shuttle.Shuttle("bid_book", value.m_bidBook);
    }
  };
}

#endif
