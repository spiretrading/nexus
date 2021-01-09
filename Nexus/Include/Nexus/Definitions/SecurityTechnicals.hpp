#ifndef NEXUS_SECURITY_TECHNICALS_HPP
#define NEXUS_SECURITY_TECHNICALS_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Money.hpp"

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
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::SecurityTechnicals> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::SecurityTechnicals& value,
        unsigned int version) {
      shuttle.Shuttle("volume", value.m_volume);
      shuttle.Shuttle("high", value.m_high);
      shuttle.Shuttle("low", value.m_low);
      shuttle.Shuttle("open", value.m_open);
      shuttle.Shuttle("close", value.m_close);
    }
  };
}

#endif
