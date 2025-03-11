#ifndef NEXUS_MARKET_DATA_TYPE_HPP
#define NEXUS_MARKET_DATA_TYPE_HPP
#include <Beam/Collections/Enum.hpp>
#include <Beam/Collections/EnumSet.hpp>
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /* Lists the types market data available to subscribe to. */
  BEAM_ENUM(MarketDataType,

    /** A TimeAndSale. */
    TIME_AND_SALE,

    /** A BookQuote. */
    BOOK_QUOTE,

    /** A MarketQuote. */
    MARKET_QUOTE,

    /** A BboQuote. */
    BBO_QUOTE,

    /** An OrderImbalance. */
    ORDER_IMBALANCE);

  /** Stores a set of MarketDataTypes. */
  using MarketDataTypeSet = Beam::EnumSet<MarketDataType>;

  /** Returns a static type's MarketDataType. */
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
}

#endif
