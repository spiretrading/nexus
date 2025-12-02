#ifndef NEXUS_SECURITY_TECHNICALS_HPP
#define NEXUS_SECURITY_TECHNICALS_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"

namespace Nexus {

  /** Stores various technical details about a Security. */
  struct SecurityTechnicals {

    /** The day's volume. */
    Quantity m_volume;

    /** The day's highest TimeAndSale price. */
    Money m_high;

    /** The day's lowest TimeAndSale price. */
    Money m_low;

    /** The day's opening price. */
    Money m_open;

    /** The previous day's closing price. */
    Money m_close;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const SecurityTechnicals& value) {
    return out << '(' << value.m_volume << ' ' << value.m_high << ' ' <<
      value.m_low << ' ' << value.m_open << ' ' << value.m_close  << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::SecurityTechnicals> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::SecurityTechnicals& value,
        unsigned int version) const {
      shuttle.shuttle("volume", value.m_volume);
      shuttle.shuttle("high", value.m_high);
      shuttle.shuttle("low", value.m_low);
      shuttle.shuttle("open", value.m_open);
      shuttle.shuttle("close", value.m_close);
    }
  };
}

#endif
