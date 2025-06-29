#ifndef NEXUS_ORDER_TYPE_HPP
#define NEXUS_ORDER_TYPE_HPP
#include <ostream>
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
    BOOST_THROW_EXCEPTION(std::runtime_error("OrderType not found: " +
      std::to_string(static_cast<int>(value))));
  }

  inline std::ostream& operator <<(std::ostream& out, OrderType value) {
    if(value == OrderType::MARKET) {
      return out << "MARKET";
    } else if(value == OrderType::LIMIT) {
      return out << "LIMIT";
    } else if(value == OrderType::PEGGED) {
      return out << "PEGGED";
    } else if(value == OrderType::STOP) {
      return out << "STOP";
    }
    return out << "NONE";
  }
}

#endif
