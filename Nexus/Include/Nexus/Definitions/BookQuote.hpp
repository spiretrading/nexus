#ifndef NEXUS_BOOK_QUOTE_HPP
#define NEXUS_BOOK_QUOTE_HPP
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

    /** Whether the MPID is the Market's primary participant. */
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
   * Tests if two BookQuote's represent the same listing, that is they share
   * the same side, price and MPID.
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

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::BookQuote> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::BookQuote& value,
        unsigned int version) const {
      shuttle.Shuttle("mpid", value.m_mpid);
      shuttle.Shuttle("is_primary_mpid", value.m_is_primary_mpid);
      shuttle.Shuttle("venue", value.m_venue);
      shuttle.Shuttle("quote", value.m_quote);
      shuttle.Shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
