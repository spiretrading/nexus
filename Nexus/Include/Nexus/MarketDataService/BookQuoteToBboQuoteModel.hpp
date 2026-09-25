#ifndef NEXUS_BOOK_QUOTE_TO_BBO_QUOTE_MODEL_HPP
#define NEXUS_BOOK_QUOTE_TO_BBO_QUOTE_MODEL_HPP
#include <algorithm>
#include <vector>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"

namespace Nexus {

  /** Maintains a BboQuote from a stream of BookQuote updates. */
  class BookQuoteToBboQuoteModel {
    public:
      BookQuoteToBboQuoteModel() = default;

      /** Returns the current BboQuote. */
      const BboQuote& get_bbo() const;

      /**
       * Updates the model with a BookQuote.
       * @param quote The BookQuote update.
       * @return <code>true</code> iff the BboQuote changed.
       */
      bool update(const BookQuote& quote);

    private:
      std::vector<BookQuote> m_bids;
      std::vector<BookQuote> m_asks;
      BboQuote m_bbo;

      bool recompute_bbo(Side side, boost::posix_time::ptime timestamp);
  };

  inline const BboQuote& BookQuoteToBboQuoteModel::get_bbo() const {
    return m_bbo;
  }

  inline bool BookQuoteToBboQuoteModel::update(const BookQuote& quote) {
    auto& book = pick(quote.m_quote.m_side, m_asks, m_bids);
    auto [i, j] = std::ranges::equal_range(
      book, quote, [&] (const auto& lhs, const auto& rhs) {
        return offer_comparator(
          quote.m_quote.m_side, lhs.m_quote.m_price, rhs.m_quote.m_price) > 0;
      });
    auto k = std::ranges::find_if(i, j, [&] (const auto& entry) {
      return entry.m_venue == quote.m_venue && entry.m_mpid == quote.m_mpid;
    });
    if(quote.m_quote.m_size == 0) {
      if(k != j) {
        book.erase(k);
      } else {
        return false;
      }
    } else if(k != j) {
      *k = quote;
    } else {
      book.insert(j, quote);
    }
    auto& bbo = pick(quote.m_quote.m_side, m_bbo.m_ask, m_bbo.m_bid);
    if(bbo.m_size != 0 && offer_comparator(
        quote.m_quote.m_side, quote.m_quote.m_price, bbo.m_price) > 0) {
      return false;
    }
    return recompute_bbo(quote.m_quote.m_side, quote.m_timestamp);
  }

  inline bool BookQuoteToBboQuoteModel::recompute_bbo(
      Side side, boost::posix_time::ptime timestamp) {
    auto& book = pick(side, m_asks, m_bids);
    auto& bbo = pick(side, m_bbo.m_ask, m_bbo.m_bid);
    auto quote = Quote(Money::ZERO, 0, side);
    if(!book.empty()) {
      quote.m_price = book.back().m_quote.m_price;
      for(auto i = book.rbegin();
          i != book.rend() && i->m_quote.m_price == quote.m_price; ++i) {
        quote.m_size += i->m_quote.m_size;
      }
    }
    if(quote == bbo) {
      return false;
    }
    bbo = quote;
    m_bbo.m_timestamp = timestamp;
    return true;
  }
}

#endif
