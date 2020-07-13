#ifndef NEXUS_MARKET_QUOTE_HPP
#define NEXUS_MARKET_QUOTE_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Quote.hpp"

namespace Nexus {

  /** Stores the best bid and ask Quotes for a particular market. */
  struct MarketQuote {

    /** The Market listing this Quote. */
    MarketCode m_market;

    /** The bid. */
    Quote m_bid;

    /** The ask. */
    Quote m_ask;

    /** The timestamp. */
    boost::posix_time::ptime m_timestamp;

    /** Constructs a MarketQuote with 0 size and price. */
    MarketQuote();

    /**
     * Constructs an MarketQuote.
     * @param market The Market listing this Quote.
     * @param bid The Market's bid.
     * @param ask The Market's ask.
     * @param timestamp The timestamp.
     */
    MarketQuote(MarketCode market, Quote bid, Quote ask,
      boost::posix_time::ptime timestamp);

    /**
     * Tests if two MarketQuote are equal.
     * @param rhs The right hand side of the equality.
     * @return <code>true</code> iff <i>this</i> is equal to <i>rhs</i>.
     */
    bool operator ==(const MarketQuote& rhs) const;

    /**
     * Tests if two MarketQuote are not equal.
     * @param rhs The right hand side of the equality.
     * @return <code>true</code> iff <i>this</i> is not equal to <i>rhs</i>.
     */
    bool operator !=(const MarketQuote& rhs) const;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const MarketQuote& value) {
    return out << "(" << value.m_market << " " << value.m_bid << " " <<
      value.m_ask << " " << value.m_timestamp << ")";
  }

  inline MarketQuote::MarketQuote() {
    m_bid.m_side = Side::BID;
    m_ask.m_side = Side::ASK;
  }

  inline MarketQuote::MarketQuote(MarketCode market, Quote bid,
      Quote ask, boost::posix_time::ptime timestamp)
      : m_market(market),
        m_bid(std::move(bid)),
        m_ask(std::move(ask)),
        m_timestamp(timestamp) {
    assert(m_bid.m_side == Side::BID);
    assert(m_ask.m_side == Side::ASK);
  }

  inline bool MarketQuote::operator ==(const MarketQuote& rhs) const {
    return m_market == rhs.m_market && m_bid == rhs.m_bid &&
      m_ask == rhs.m_ask && m_timestamp == rhs.m_timestamp;
  }

  inline bool MarketQuote::operator !=(const MarketQuote& rhs) const {
    return !(*this == rhs);
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::MarketQuote> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::MarketQuote& value,
        unsigned int version) {
      shuttle.Shuttle("market", value.m_market);
      shuttle.Shuttle("bid", value.m_bid);
      shuttle.Shuttle("ask", value.m_ask);
      shuttle.Shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
