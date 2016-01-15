#ifndef NEXUS_MARKETDATASECURITYSNAPSHOT_HPP
#define NEXUS_MARKETDATASECURITYSNAPSHOT_HPP
#include <bitset>
#include <unordered_map>
#include <vector>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Collections/EnumSet.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \enum MarketDataType
      \brief Lists the types market data available to subscribe to.
   */
  BEAM_ENUM(MarketDataType,

    //! A TimeAndSale.
    TIME_AND_SALE,

    //! A BookQuote.
    BOOK_QUOTE,

    //! A MarketQuote.
    MARKET_QUOTE,

    //! A BboQuote.
    BBO_QUOTE,

    //! An OrderImbalance.
    ORDER_IMBALANCE);

  //! Stores a set of MarketDataTypes.
  using MarketDataTypeSet = Beam::EnumSet<MarketDataType>;

  /*! \struct SecuritySnapshot
      \brief Stores a market data snapshot of a Security.
   */
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

  //! Returns a static type's MarketDataType.
  template<typename T>
  MarketDataType GetMarketDataType() {
    if(std::is_same<T, TimeAndSale>::value) {
      return MarketDataType::TIME_AND_SALE;
    } else if(std::is_same<T, BookQuote>::value) {
      return MarketDataType::BOOK_QUOTE;
    } else if(std::is_same<T, MarketQuote>::value) {
      return MarketDataType::MARKET_QUOTE;
    } else if(std::is_same<T, BboQuote>::value) {
      return MarketDataType::BBO_QUOTE;
    } else if(std::is_same<T, OrderImbalance>::value) {
      return MarketDataType::ORDER_IMBALANCE;
    }
    return MarketDataType::NONE;
  }

  inline SecuritySnapshot::SecuritySnapshot(const Security& security)
      : m_security(security) {}
}
}

namespace Beam {
namespace Serialization {
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
}

#endif
