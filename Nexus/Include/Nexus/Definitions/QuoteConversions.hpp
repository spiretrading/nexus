#ifndef NEXUS_QUOTE_CONVERSIONS_HPP
#define NEXUS_QUOTE_CONVERSIONS_HPP
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/MarketQuote.hpp"

namespace Nexus {

  /** Stores a bid/ask pair of BookQuotes. */
  struct BookQuotePair {

    /** The BookQuote on the ASK Side. */
    BookQuote m_ask;

    /** The BookQuote on the BID Side. */
    BookQuote m_bid;
  };

  /**
   * Converts a MarketQuote into a BookQuotePair.
   * @param quote The MarketQuote to convert.
   * @return A BookQuotePair representing the <i>quote</i>.
   */
  inline BookQuotePair ToBookQuotePair(const MarketQuote& quote) {
    return BookQuotePair(BookQuote(quote.m_market.GetData(), true,
      quote.m_market, quote.m_ask, quote.m_timestamp),
      BookQuote(quote.m_market.GetData(), true, quote.m_market, quote.m_bid,
      quote.m_timestamp));
  }
}

#endif
