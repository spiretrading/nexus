#ifndef NEXUS_QUOTE_HPP
#define NEXUS_QUOTE_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Side.hpp"

namespace Nexus {

  /** Represents a quote on a security. */
  struct Quote {

    /** The quote's price. */
    Money m_price;

    /** The quote's size. */
    Quantity m_size;

    /** The quote's Side. */
    Side m_side;

    /** Constructs an uninitialized Quote. */
    Quote();

    /**
     * Constructs a Quote.
     * @param price The quote's price.
     * @param size The quote's size.
     * @param side The quote's side.
     */
    Quote(Money price, Quantity size, Side side);

    bool operator ==(const Quote& rhs) const = default;
  };

  /**
   * Tests if one Quote should be listed before another.
   * @param lhs The left hand side of the comparator.
   * @param rhs The right hand side of the comparator.
   * @return <code>true</code> iff <i>lhs</i> should be listed before <i>rhs</i>
   *         according to its Side and price.
   */
  inline bool QuoteListingComparator(const Quote& lhs, const Quote& rhs) {
    if(lhs.m_side == Side::ASK) {
      return lhs.m_price < rhs.m_price;
    } else {
      return lhs.m_price > rhs.m_price;
    }
  }

  /**
   * Returns the precedence an offer has based on its Side and price.
   * @param side The Side of the price offers.
   * @param lhs The left hand side of the comparator.
   * @param rhs The right hand side of the comparator.
   * @return Less than 0: <i>lhs</i> has greater precedence than <i>rhs</i>.
   *         Equal to 0: <i>lhs</i> is equal to <i>rhs</i>.
   *         Greater than 0: <i>lhs</i> has lower precedence than <i>rhs</i>.
   */
  inline int OfferComparator(Side side, Money lhs, Money rhs) {
    if(side == Side::ASK) {
      if(lhs < rhs) {
        return -1;
      } else if(lhs == rhs) {
        return 0;
      }
      return 1;
    }
    if(lhs > rhs) {
      return -1;
    } else if(lhs == rhs) {
      return 0;
    }
    return 1;
  }

  inline Quote::Quote()
    : m_price(Money::ZERO),
      m_size(0),
      m_side(Side::NONE) {}

  inline Quote::Quote(Money price, Quantity size, Side side)
    : m_price(price),
      m_size(size),
      m_side(side) {}

  inline std::ostream& operator <<(std::ostream& out, const Quote& quote) {
    return out << "(" << quote.m_price << " " << quote.m_size << " " <<
      quote.m_side << ")";
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::Quote> {
    template<typename Shuttler>
    void operator ()(
        Shuttler& shuttle, Nexus::Quote& value, unsigned int version) {
      shuttle.Shuttle("price", value.m_price);
      shuttle.Shuttle("size", value.m_size);
      shuttle.Shuttle("side", value.m_side);
    }
  };
}

#endif
