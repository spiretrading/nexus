#ifndef NEXUS_TIME_AND_SALE_HPP
#define NEXUS_TIME_AND_SALE_HPP
#include <ostream>
#include <string>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Money.hpp"

namespace Nexus {
namespace Details {
  BEAM_ENUM(ConditionTypeDefinition,

    /** A regular trade. */
    REGULAR,

    /** An opening print. */
    OPEN,

    /** A closing print. */
    CLOSE);
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

      bool operator ==(const Condition& condition) const = default;
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
    std::string m_marketCenter;

    /** The buyer MPID. */
    std::string m_buyerMpid;

    /** The seller MPID. */
    std::string m_sellerMpid;

    bool operator ==(const TimeAndSale& timeAndSale) const = default;
  };

  inline std::ostream& operator <<(std::ostream& out,
      TimeAndSale::Condition::Type value) {
    if(value == TimeAndSale::Condition::Type::REGULAR) {
      return out << "@";
    } else if(value == TimeAndSale::Condition::Type::OPEN) {
      return out << "OPEN";
    } else if(value == TimeAndSale::Condition::Type::CLOSE) {
      return out << "CLOSE";
    }
    return out << "NONE";
  }

  inline std::ostream& operator <<(std::ostream& out,
      const TimeAndSale::Condition& value) {
    return out << "(" << value.m_type << " " << value.m_code << ")";
  }

  inline std::ostream& operator <<(std::ostream& out,
      const TimeAndSale& value) {
    return out << "(" << value.m_timestamp << " " << value.m_price << " " <<
      value.m_size << " " << value.m_condition << " " << value.m_marketCenter <<
      value.m_buyerMpid << " " << value.m_sellerMpid << ")";
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::TimeAndSale::Condition> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::TimeAndSale::Condition& value,
        unsigned int version) {
      shuttle.Shuttle("type", value.m_type);
      shuttle.Shuttle("code", value.m_code);
    }
  };

  template<>
  struct Shuttle<Nexus::TimeAndSale> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::TimeAndSale& value,
        unsigned int version) {
      shuttle.Shuttle("timestamp", value.m_timestamp);
      shuttle.Shuttle("price", value.m_price);
      shuttle.Shuttle("size", value.m_size);
      shuttle.Shuttle("condition", value.m_condition);
      shuttle.Shuttle("market_center", value.m_marketCenter);
      shuttle.Shuttle("buyer_mpid", value.m_buyerMpid);
      shuttle.Shuttle("seller_mpid", value.m_sellerMpid);
    }
  };
}

#endif
