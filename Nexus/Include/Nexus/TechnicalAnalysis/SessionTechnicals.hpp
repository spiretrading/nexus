#ifndef NEXUS_SESSION_TECHNICALS_HPP
#define NEXUS_SESSION_TECHNICALS_HPP
#include <algorithm>
#include <ostream>
#include <string_view>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"

namespace Nexus {

  /** Stores the technical figures for a Ticker's trading session. */
  struct SessionTechnicals {

    /** The first trade of the session. */
    boost::optional<Money> m_open;

    /** The last trade of the previous session. */
    boost::optional<Money> m_previous_close;

    /** The highest trade of the session. */
    boost::optional<Money> m_high;

    /** The lowest trade of the session. */
    boost::optional<Money> m_low;

    /** The total volume traded during the session. */
    Quantity m_volume;

    bool operator ==(const SessionTechnicals&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const SessionTechnicals& value) {
    auto print = [&] (const auto& price) {
      if(price) {
        out << *price;
      } else {
        out << "none";
      }
    };
    out << '(';
    print(value.m_open);
    out << ' ';
    print(value.m_previous_close);
    out << ' ';
    print(value.m_high);
    out << ' ';
    print(value.m_low);
    return out << ' ' << value.m_volume << ')';
  }

  /**
   * Updates a SessionTechnicals with a trade from a Ticker's session.
   * @param technicals The technicals to update.
   * @param time_and_sale The trade to fold in.
   * @param market_center The venue's market center.
   */
  inline void update(SessionTechnicals& technicals,
      const TimeAndSale& time_and_sale, std::string_view market_center) {
    if(technicals.m_high) {
      technicals.m_high = std::max(*technicals.m_high, time_and_sale.m_price);
      technicals.m_low = std::min(*technicals.m_low, time_and_sale.m_price);
    } else {
      technicals.m_high = time_and_sale.m_price;
      technicals.m_low = time_and_sale.m_price;
    }
    if(!technicals.m_open && time_and_sale.m_market_center == market_center) {
      technicals.m_open = time_and_sale.m_price;
    }
    technicals.m_volume += time_and_sale.m_size;
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::SessionTechnicals> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::SessionTechnicals& value,
        unsigned int version) const {
      shuttle.shuttle("open", value.m_open);
      shuttle.shuttle("previous_close", value.m_previous_close);
      shuttle.shuttle("high", value.m_high);
      shuttle.shuttle("low", value.m_low);
      shuttle.shuttle("volume", value.m_volume);
    }
  };
}

#endif
