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

      /** Constructs a default Condition. */
      Condition() = default;

      /**
       * Constructs a Condition.
       * @param type The type of condition.
       * @param code The condition's code.
       */
      Condition(Type type, std::string code);

      /** Tests a Condition for equality. */
      bool operator ==(const Condition& condition) const;

      /** Tests a Condition for inequality. */
      bool operator !=(const Condition& condition) const;
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

    /** Constructs an uninitialized TimeAndSale. */
    TimeAndSale() = default;

    /**
     * Constructs a TimeAndSale.
     * @param timestamp The time of the transaction.
     * @param price The transaction's price.
     * @param size The size of the transaction.
     * @param condition The sale condition.
     * @param marketCenter The market center.
     */
    TimeAndSale(boost::posix_time::ptime timestamp, Money price,
      Quantity size, Condition condition, std::string marketCenter);

    /** Tests a TimeAndSale for equality. */
    bool operator ==(const TimeAndSale& timeAndSale) const;

    /** Tests a TimeAndSale for inequality. */
    bool operator !=(const TimeAndSale& timeAndSale) const;
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
      ")";
  }

  inline TimeAndSale::Condition::Condition(Type type, std::string code)
    : m_type(type),
      m_code(std::move(code)) {}

  inline TimeAndSale::TimeAndSale(boost::posix_time::ptime timestamp,
    Money price, Quantity size, Condition condition, std::string marketCenter)
    : m_timestamp(timestamp),
      m_price(price),
      m_size(size),
      m_condition(std::move(condition)),
      m_marketCenter(std::move(marketCenter)) {}

  inline bool TimeAndSale::operator ==(const TimeAndSale& timeAndSale) const {
    return m_timestamp == timeAndSale.m_timestamp &&
      m_price == timeAndSale.m_price && m_size == timeAndSale.m_size &&
      m_condition == timeAndSale.m_condition &&
      m_marketCenter == timeAndSale.m_marketCenter;
  }

  inline bool TimeAndSale::operator !=(const TimeAndSale& timeAndSale) const {
    return !(*this == timeAndSale);
  }

  inline bool TimeAndSale::Condition::operator ==(
      const Condition& condition) const {
    return m_type == condition.m_type && m_code == condition.m_code;
  }

  inline bool TimeAndSale::Condition::operator !=(
      const Condition& condition) const {
    return !(*this == condition);
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
    }
  };
}

#endif
