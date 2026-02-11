#ifndef NEXUS_DEFINITIONS_ORDER_TYPE_HPP
#define NEXUS_DEFINITIONS_ORDER_TYPE_HPP
#include <stdexcept>
#include <string>
#include <Beam/Collections/Enum.hpp>
#include <boost/throw_exception.hpp>

namespace Nexus {

  /** Represents the types of Orders. */
  BEAM_ENUM(OrderType,

    /** Market order. */
    MARKET,

    /** Limit order. */
    LIMIT,

    /** Pegged order. */
    PEGGED,

    /** Stop order. */
    STOP);

  /** Returns the single character FIX representation of an OrderType. */
  [[nodiscard]]
  inline char to_char(OrderType value) {
    if(value == OrderType::MARKET) {
      return '1';
    } else if(value == OrderType::LIMIT) {
      return '2';
    } else if(value == OrderType::PEGGED) {
      return 'P';
    } else if(value == OrderType::STOP) {
      return '3';
    }
    boost::throw_with_location(std::runtime_error(
      "OrderType not found: " + std::to_string(static_cast<int>(value))));
  }
}

#endif
