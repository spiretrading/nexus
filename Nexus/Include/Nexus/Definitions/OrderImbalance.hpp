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

    /** Constructs an uninitialized OrderImbalance. */
    OrderImbalance() = default;

    /**
     * Constructs an OrderImbalance.
     * @param security The Security with the order imbalance.
     * @param side The Side of the imbalance.
     * @param size The size of the imbalance.
     * @param referencePrice The price of the imbalance.
     * @param timestamp The imbalance's time stamp.
     */
    OrderImbalance(Security security, Side side, Quantity size,
      Money referencePrice, boost::posix_time::ptime timestamp);

    /**
     * Returns <code>true</code> iff two OrderImbalances are equal.
     * @param orderImbalance The OrderImbalance to compare for equality.
     * @return <code>true</code> iff the two OrderImbalances are equal.
     */
    bool operator ==(const OrderImbalance& orderImbalance) const;

    /**
     * Returns <code>true</code> iff two OrderImbalances are not equal.
     * @param orderImbalance The OrderImbalance to compare for inequality.
     * @return <code>true</code> iff the two OrderImbalances are not equal.
     */
    bool operator !=(const OrderImbalance& orderImbalance) const;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const OrderImbalance& value) {
    return out << "(" << value.m_security << " " << value.m_side << " " <<
      value.m_size << " " << value.m_referencePrice << " " <<
      value.m_timestamp << ")";
  }

  inline OrderImbalance::OrderImbalance(Security security, Side side,
    Quantity size, Money referencePrice, boost::posix_time::ptime timestamp)
    : m_security(std::move(security)),
      m_side(side),
      m_size(size),
      m_referencePrice(referencePrice),
      m_timestamp(timestamp) {}

  inline bool OrderImbalance::operator ==(
      const OrderImbalance& orderImbalance) const {
    return m_security == orderImbalance.m_security &&
      m_side == orderImbalance.m_side &&
      m_size == orderImbalance.m_size &&
      m_referencePrice == orderImbalance.m_referencePrice &&
      m_timestamp == orderImbalance.m_timestamp;
  }

  inline bool OrderImbalance::operator !=(
      const OrderImbalance& orderImbalance) const {
    return !(*this == orderImbalance);
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
