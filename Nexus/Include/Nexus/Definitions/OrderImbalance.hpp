#ifndef NEXUS_ORDER_IMBALANCE_HPP
#define NEXUS_ORDER_IMBALANCE_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Ticker.hpp"

namespace Nexus {

  /** Represents an order imbalance. */
  struct OrderImbalance {

    /** The Ticker with the order imbalance. */
    Ticker m_ticker;

    /** The Side of the imbalance. */
    Side m_side;

    /** The size of the imbalance. */
    Quantity m_size;

    /** The reference price of the imbalance. */
    Money m_reference_price;

    /** The timestamp. */
    boost::posix_time::ptime m_timestamp;

    bool operator ==(const OrderImbalance&) const = default;
  };
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::OrderImbalance> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::OrderImbalance& value, unsigned int version) const {
      shuttle.shuttle("ticker", value.m_ticker);
      shuttle.shuttle("side", value.m_side);
      shuttle.shuttle("size", value.m_size);
      shuttle.shuttle("reference_price", value.m_reference_price);
      shuttle.shuttle("timestamp", value.m_timestamp);
    }
  };
}

template<>
class std::formatter<Nexus::OrderImbalance> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::OrderImbalance& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {} {} {} {})", value.m_ticker,
          value.m_side, value.m_size, value.m_reference_price,
          Beam::to_string(value.m_timestamp)), context);
    }
};

namespace Nexus {
  inline std::ostream& operator <<(
      std::ostream& out, const OrderImbalance& value) {
    auto database = static_cast<const VenueDatabase*>(
      out.pword(Beam::ScopedStreamManipulator<VenueDatabase>::ID));
    if(database) {
      return out <<
        std::format(make_locale(out.getloc(), *database), "{}", value);
    }
    return out << std::format(out.getloc(), "{}", value);
  }
}

#endif
