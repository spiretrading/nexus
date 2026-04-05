#ifndef NEXUS_BBO_QUOTE_MODEL_HPP
#define NEXUS_BBO_QUOTE_MODEL_HPP
#include <algorithm>
#include <vector>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"

namespace Nexus {

  /** Maintains a BboQuote from a stream of BookQuote updates. */
  class BboQuoteModel {
    public:

      /** Constructs a BboQuoteModel. */
      BboQuoteModel() = default;

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

  inline const BboQuote& BboQuoteModel::get_bbo() const {
    return m_bbo;
  }

  inline bool BboQuoteModel::update(const BookQuote& quote) {
    auto& book = pick(quote.m_quote.m_side, m_asks, m_bids);
    auto [lower, upper] = std::ranges::equal_range(book, quote,
      [&](const auto& lhs, const auto& rhs) {
        return offer_comparator(
          quote.m_quote.m_side, lhs.m_quote.m_price, rhs.m_quote.m_price) > 0;
      });
    auto i = std::find_if(lower, upper, [&](const auto& entry) {
      return entry.m_mpid == quote.m_mpid;
    });
    if(quote.m_quote.m_size == 0) {
      if(i != upper) {
        book.erase(i);
      } else {
        return false;
      }
    } else if(i != upper) {
      *i = quote;
    } else {
      book.insert(upper, quote);
    }
    auto& bbo = pick(quote.m_quote.m_side, m_bbo.m_ask, m_bbo.m_bid);
    if(bbo.m_size != 0 && offer_comparator(quote.m_quote.m_side,
        quote.m_quote.m_price, bbo.m_price) > 0) {
      return false;
    }
    return recompute_bbo(quote.m_quote.m_side, quote.m_timestamp);
  }

  inline bool BboQuoteModel::recompute_bbo(
      Side side, boost::posix_time::ptime timestamp) {
    auto& book = pick(side, m_asks, m_bids);
    auto& bbo = pick(side, m_bbo.m_ask, m_bbo.m_bid);
    if(book.empty()) {
      if(bbo.m_size == 0) {
        return false;
      }
      bbo = Quote(Money::ZERO, 0, side);
      m_bbo.m_timestamp = timestamp;
      return true;
    }
    auto& back = book.back().m_quote;
    if(bbo.m_price == back.m_price && bbo.m_size == back.m_size) {
      return false;
    }
    auto best = book.rbegin();
    auto i = best + 1;
    while(i != book.rend() && i->m_quote.m_price == best->m_quote.m_price) {
      if(i->m_quote.m_size > best->m_quote.m_size) {
        best = i;
      }
      ++i;
    }
    if(best != book.rbegin()) {
      std::iter_swap(best, book.rbegin());
    }
    bbo = book.back().m_quote;
    m_bbo.m_timestamp = timestamp;
    return true;
  }
}

#endif
