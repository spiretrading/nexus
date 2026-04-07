#ifndef NEXUS_MARKET_DATA_TYPE_HPP
#define NEXUS_MARKET_DATA_TYPE_HPP
#include <ostream>
#include <type_traits>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Collections/EnumSet.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"

namespace Nexus {

  /* Lists the types market data available to subscribe to. */
  BEAM_ENUM(MarketDataType,

    /** A TimeAndSale. */
    TIME_AND_SALE,

    /** A BookQuote. */
    BOOK_QUOTE,

    /** A BboQuote. */
    BBO_QUOTE,

    /** An OrderImbalance. */
    ORDER_IMBALANCE);

  /** Stores a set of MarketDataTypes. */
  using MarketDataTypeSet = Beam::EnumSet<MarketDataType>;

  /** Returns a static type's MarketDataType. */
  template<typename T>
  MarketDataType get_market_data_type() {
    if constexpr(std::is_same_v<T, TimeAndSale>) {
      return MarketDataType::TIME_AND_SALE;
    } else if constexpr(std::is_same_v<T, BookQuote>) {
      return MarketDataType::BOOK_QUOTE;
    } else if constexpr(std::is_same_v<T, BboQuote>) {
      return MarketDataType::BBO_QUOTE;
    } else if constexpr(std::is_same_v<T, OrderImbalance>) {
      return MarketDataType::ORDER_IMBALANCE;
    }
    return MarketDataType::NONE;
  }
}

#endif
