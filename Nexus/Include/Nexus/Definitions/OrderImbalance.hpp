#ifndef NEXUS_ORDER_IMBALANCE_HPP
#define NEXUS_ORDER_IMBALANCE_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"

namespace Nexus {

  /** Represents an order imbalance. */
  struct OrderImbalance {

    /** The Security with the order imbalance. */
    Security m_security;

    /** The Side of the imbalance. */
    Side m_side;

    /** The size of the imbalance. */
    Quantity m_size;

    /** The reference price of the imbalance. */
    Money m_referencePrice;

    /** The timestamp. */
    boost::posix_time::ptime m_timestamp;

    bool operator ==(const OrderImbalance& orderImbalance) const = default;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const OrderImbalance& value) {
    return out << "(" << value.m_security << " " << value.m_side << " " <<
      value.m_size << " " << value.m_referencePrice << " " <<
      value.m_timestamp << ")";
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::OrderImbalance> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::OrderImbalance& value,
        unsigned int version) {
      shuttle.Shuttle("security", value.m_security);
      shuttle.Shuttle("side", value.m_side);
      shuttle.Shuttle("size", value.m_size);
      shuttle.Shuttle("reference_price", value.m_referencePrice);
      shuttle.Shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
