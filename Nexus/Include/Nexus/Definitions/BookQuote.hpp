#ifndef NEXUS_DEFINITIONS_BOOK_QUOTE_HPP
#define NEXUS_DEFINITIONS_BOOK_QUOTE_HPP
#include <string>
#include <ostream>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Quote.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Stores a Quote booked by an MPID for a particular market. */
  struct BookQuote {

    /** The quote's MPID. */
    std::string m_mpid;

    /** Whether the MPID is the venue's primary participant. */
    bool m_is_primary_mpid = false;

    /** The venue this quote comes from. */
    Venue m_venue;

    /** The quote. */
    Quote m_quote;

    /** The timestamp. */
    boost::posix_time::ptime m_timestamp;

    bool operator ==(const BookQuote&) const = default;
  };

  /**
   * Tests if two BookQuotes represent the same listing, that is they share the
   * same side, price and MPID.
   */
  inline bool is_same_listing(const BookQuote& lhs, const BookQuote& rhs) {
    return std::tie(lhs.m_quote.m_side, lhs.m_quote.m_price, lhs.m_mpid) ==
      std::tie(rhs.m_quote.m_side, rhs.m_quote.m_price, rhs.m_mpid);
  }

  /**
   * Tests if one BookQuote should be listed before another.
   * @param lhs The left hand side of the comparator.
   * @param rhs The right hand side of the comparator.
   * @return <code>true</code> iff <i>lhs</i> should be listed before <i>rhs</i>
   *         according to its Side and price.
   */
  inline bool listing_comparator(const BookQuote& lhs, const BookQuote& rhs) {
    if(lhs.m_quote.m_price != rhs.m_quote.m_price) {
      return listing_comparator(lhs.m_quote, rhs.m_quote);
    }
    return lhs.m_mpid < rhs.m_mpid;
  }

  inline std::ostream& operator <<(std::ostream& out, const BookQuote& quote) {
    return out << '(' << quote.m_mpid << ' ' << quote.m_is_primary_mpid <<
      ' ' << quote.m_venue << ' ' << quote.m_quote << ' ' <<
      quote.m_timestamp << ')';
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::BookQuote> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::BookQuote& value, unsigned int version) const {
      shuttle.shuttle("mpid", value.m_mpid);
      shuttle.shuttle("is_primary_mpid", value.m_is_primary_mpid);
      shuttle.shuttle("venue", value.m_venue);
      shuttle.shuttle("quote", value.m_quote);
      shuttle.shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
