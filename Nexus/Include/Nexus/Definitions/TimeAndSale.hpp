#ifndef NEXUS_TIME_AND_SALE_HPP
#define NEXUS_TIME_AND_SALE_HPP
#include <ostream>
#include <string>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"

namespace Nexus {
namespace Details {
  BEAM_BASIC_ENUM(ConditionTypeDefinition,

    /** A regular trade. */
    REGULAR,

    /** An opening print. */
    OPEN,

    /** A closing print. */
    CLOSE);

  inline std::ostream& operator <<(
      std::ostream& out, ConditionTypeDefinition::Type value) {
    if(value == ConditionTypeDefinition::REGULAR) {
      return out << "@";
    } else if(value == ConditionTypeDefinition::OPEN) {
      return out << "OPEN";
    } else if(value == ConditionTypeDefinition::CLOSE) {
      return out << "CLOSE";
    }
    return out << "NONE";
  }
}

  /** Stores a record of a transaction on a Security. */
  struct TimeAndSale {

    /** The sale's condition. */
    struct Condition {

      /** Lists types of conditions shared amongst multiple markets. */
      using Type = Details::ConditionTypeDefinition;

      /** The type of condition. */
      Type m_type;

      /** The condition's code. */
      std::string m_code;

      bool operator ==(const Condition&) const = default;
    };

    /** The time of the transaction. */
    boost::posix_time::ptime m_timestamp;

    /** The transaction's price. */
    Money m_price;

    /** The size of the transaction. */
    Quantity m_size;

    /** The sale condition. */
    Condition m_condition;

    /** The market center. */
    std::string m_market_center;

    /** The buyer MPID. */
    std::string m_buyer_mpid;

    /** The seller MPID. */
    std::string m_seller_mpid;

    bool operator ==(const TimeAndSale&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, TimeAndSale::Condition::Type value) {
    return out << TimeAndSale::Condition::Type::Type(value);
  }

  inline std::ostream& operator <<(std::ostream& out,
      const TimeAndSale::Condition& value) {
    return out << '(' << value.m_type << ' ' << value.m_code << ')';
  }

  inline std::ostream& operator <<(std::ostream& out,
      const TimeAndSale& value) {
    return out << '(' << value.m_timestamp << ' ' << value.m_price << ' ' <<
      value.m_size << ' ' << value.m_condition << ' ' <<
      value.m_market_center << ' ' << value.m_buyer_mpid << ' ' <<
      value.m_seller_mpid << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::TimeAndSale::Condition> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::TimeAndSale::Condition& value,
        unsigned int version) const {
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("code", value.m_code);
    }
  };

  template<>
  struct Shuttle<Nexus::TimeAndSale> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::TimeAndSale& value, unsigned int version) const {
      shuttle.shuttle("timestamp", value.m_timestamp);
      shuttle.shuttle("price", value.m_price);
      shuttle.shuttle("size", value.m_size);
      shuttle.shuttle("condition", value.m_condition);
      shuttle.shuttle("market_center", value.m_market_center);
      shuttle.shuttle("buyer_mpid", value.m_buyer_mpid);
      shuttle.shuttle("seller_mpid", value.m_seller_mpid);
    }
  };
}

#endif
