#ifndef NEXUS_BOOK_QUOTE_HPP
#define NEXUS_BOOK_QUOTE_HPP
#include <string>
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Quote.hpp"

namespace Nexus {

  /** Stores a Quote booked by an MPID for a particular market. */
  struct BookQuote {

    /** The quote's MPID. */
    std::string m_mpid;

    /** Whether the MPID is the Market's primary participant. */
    bool m_isPrimaryMpid;

    /** The market this quote comes from. */
    MarketCode m_market;

    /** The quote. */
    Quote m_quote;

    /** The timestamp. */
    boost::posix_time::ptime m_timestamp;

    /** Constructs an uninitialized BookQuote. */
    BookQuote() = default;

    /**
     * Constructs a BookQuote.
     * @param mpid The market's MPID.
     * @param isPrimaryMpid Whether the MPID is the Market's primary
     *        participant.
     * @param market The Market this Quote comes from.
     * @param quote The Quote.
     * @param timestamp The timestamp.
     */
    BookQuote(std::string mpid, bool isPrimaryMpid, MarketCode market,
      Quote quote, boost::posix_time::ptime timestamp);

    /**
     * Tests if two BookQuotes represent the same quote.
     * @param quote The BookQuote to test.
     * @return <code>true</code> iff all but the time stamps are equal.
     */
    bool operator ==(const BookQuote& quote) const;

    /**
     * Tests if two BookQuotes represent different quotes.
     * @param quote The BookQuote to test.
     * @return <code>!(*this == quote)</code>.
     */
    bool operator !=(const BookQuote& quote) const;
  };

  /**
   * Tests if one BookQuote should be listed before another.
   * @param lhs The left hand side of the comparator.
   * @param rhs The right hand side of the comparator.
   * @return <code>true</code> iff <i>lhs</i> should be listed before <i>rhs</i>
   *         according to its Side and price.
   */
  inline bool BookQuoteListingComparator(const BookQuote& lhs,
      const BookQuote& rhs) {
    if(lhs.m_quote.m_price != rhs.m_quote.m_price) {
      return QuoteListingComparator(lhs.m_quote, rhs.m_quote);
    }
    return lhs.m_mpid < rhs.m_mpid;
  }

  inline BookQuote::BookQuote(std::string mpid, bool isPrimaryMpid,
    MarketCode market, Quote quote, boost::posix_time::ptime timestamp)
    : m_mpid(std::move(mpid)),
      m_isPrimaryMpid(isPrimaryMpid),
      m_market(market),
      m_quote(std::move(quote)),
      m_timestamp(timestamp) {}

  inline bool BookQuote::operator ==(const BookQuote& quote) const {
    return m_mpid == quote.m_mpid && m_isPrimaryMpid == quote.m_isPrimaryMpid &&
      m_market == quote.m_market && m_quote == quote.m_quote && m_timestamp ==
      quote.m_timestamp;
  }

  inline bool BookQuote::operator !=(const BookQuote& quote) const {
    return !(*this == quote);
  }

  inline std::ostream& operator <<(std::ostream& out, const BookQuote& quote) {
    return out << "(" << quote.m_mpid << " " << quote.m_isPrimaryMpid << " " <<
      quote.m_market << " " << quote.m_quote << " " << quote.m_timestamp << ")";
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::BookQuote> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::BookQuote& value,
        unsigned int version) {
      shuttle.Shuttle("mpid", value.m_mpid);
      shuttle.Shuttle("is_primary_mpid", value.m_isPrimaryMpid);
      shuttle.Shuttle("market", value.m_market);
      shuttle.Shuttle("quote", value.m_quote);
      shuttle.Shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
