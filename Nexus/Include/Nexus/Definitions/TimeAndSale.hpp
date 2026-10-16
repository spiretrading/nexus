#ifndef NEXUS_TIME_AND_SALE_HPP
#define NEXUS_TIME_AND_SALE_HPP
#include <ostream>
#include <string>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Utilities/ToString.hpp>
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
    CLOSE,

    /** A reopening auction print following a trading halt. */
    REOPEN,

    /** An auction print without a more specific auction classification. */
    AUCTION,

    /** A correction to a previously reported trade. */
    CORRECTION,

    /** A cancellation of a previously reported trade. */
    CANCELLATION);
}

  /** Stores a record of a transaction on a Ticker. */
  struct TimeAndSale {

    /** The sale's condition. */
    struct Condition {

      /** Lists types of conditions shared amongst multiple venues. */
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

template<>
class std::formatter<Nexus::Details::ConditionTypeDefinition::Type> :
    public std::formatter<std::string_view> {
  public:
    auto format(Nexus::Details::ConditionTypeDefinition::Type value,
        auto& context) const {
      auto name = [&] () -> std::string_view {
        using Type = Nexus::Details::ConditionTypeDefinition;
        switch(value) {
          case Type::REGULAR:
            return "@";
          case Type::OPEN:
            return "OPEN";
          case Type::CLOSE:
            return "CLOSE";
          case Type::REOPEN:
            return "REOPEN";
          case Type::AUCTION:
            return "AUCTION";
          case Type::CORRECTION:
            return "CORRECTION";
          case Type::CANCELLATION:
            return "CANCELLATION";
          default:
            return "NONE";
        }
      }();
      return std::formatter<std::string_view>::format(name, context);
    }
};

template<>
class std::formatter<Nexus::TimeAndSale::Condition> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::TimeAndSale::Condition& value,
        auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {})", value.m_type, value.m_code),
        context);
    }
};

template<>
class std::formatter<Nexus::TimeAndSale> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::TimeAndSale& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {} {} {} {} {} {})",
          Beam::to_string(value.m_timestamp), value.m_price, value.m_size,
          value.m_condition, value.m_market_center, value.m_buyer_mpid,
          value.m_seller_mpid), context);
    }
};

namespace Nexus::Details {
  inline std::ostream& operator <<(
      std::ostream& out, ConditionTypeDefinition::Type value) {
    return out << std::format("{}", value);
  }
}

namespace Nexus {
  inline std::ostream& operator <<(
      std::ostream& out, TimeAndSale::Condition::Type value) {
    return out << std::format("{}", value);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const TimeAndSale::Condition& value) {
    return out << std::format(out.getloc(), "{}", value);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const TimeAndSale& value) {
    return out << std::format(out.getloc(), "{}", value);
  }
}

#endif
