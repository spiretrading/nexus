#ifndef NEXUS_BBO_QUOTE_HPP
#define NEXUS_BBO_QUOTE_HPP
#include <cassert>
#include <ostream>
#include <utility>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include "Nexus/Definitions/Quote.hpp"

namespace Nexus {

  /** Stores the best bid and ask Quotes across all markets. */
  struct BboQuote {

    /** The best bid. */
    Quote m_bid;

    /** The best ask. */
    Quote m_ask;

    /** The time of the update. */
    boost::posix_time::ptime m_timestamp;

    /** Constructs an uninitialized BboQuote. */
    BboQuote() noexcept;

    /**
     * Constructs a BboQuote.
     * @param bid The best bid.
     * @param ask The best ask.
     * @param timestamp The time of the transaction.
     */
    BboQuote(Quote bid, Quote ask, boost::posix_time::ptime timestamp) noexcept;

    bool operator ==(const BboQuote&) const = default;
  };

  inline std::ostream& operator <<(std::ostream& out, const BboQuote& value) {
    return out << '(' << value.m_bid << ' ' << value.m_ask << ' ' <<
      value.m_timestamp << ')';
  }

  inline BboQuote::BboQuote() noexcept {
    m_bid.m_side = Side::BID;
    m_ask.m_side = Side::ASK;
  }

  inline BboQuote::BboQuote(
      Quote bid, Quote ask, boost::posix_time::ptime timestamp) noexcept
      : m_bid(std::move(bid)),
        m_ask(std::move(ask)),
        m_timestamp(timestamp) {
    assert(m_bid.m_side == Side::BID);
    assert(m_ask.m_side == Side::ASK);
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::BboQuote> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::BboQuote& value, unsigned int version) const {
      shuttle.shuttle("bid", value.m_bid);
      shuttle.shuttle("ask", value.m_ask);
      shuttle.shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
