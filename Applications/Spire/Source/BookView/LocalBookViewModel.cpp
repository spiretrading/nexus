#include "Spire/BookView/LocalBookViewModel.hpp"
#include "Nexus/Definitions/MarketQuote.hpp"
#include "Nexus/Definitions/QuoteConversions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

LocalBookViewModel::LocalBookViewModel(Security security,
    Definitions definitions)
    : m_security(std::move(security)),
      m_definitions(std::move(definitions)) {}

const Definitions& LocalBookViewModel::get_definitions() const {
  return m_definitions;
}

void LocalBookViewModel::update(const BboQuote& bbo) {
  m_bbo = bbo;
  m_bbo_signal(m_bbo);
}

void LocalBookViewModel::update(const BookQuote& quote) {
  update(quote, Type::BOOK_QUOTE);
}

void LocalBookViewModel::update(const MarketQuote& quote) {
  auto& previous = m_market_quotes[quote.m_market];
  auto mpid = m_definitions.get_market_database().FromCode(
    quote.m_market).m_displayName;
  if(!previous.m_market.IsEmpty()) {
    auto book_quotes = ToBookQuotePair(previous);
    book_quotes.m_ask.m_quote.m_size = 0;
    book_quotes.m_ask.m_mpid = mpid;
    update(book_quotes.m_ask, Type::MARKET_QUOTE);
    book_quotes.m_bid.m_quote.m_size = 0;
    book_quotes.m_bid.m_mpid = mpid;
    update(book_quotes.m_bid, Type::MARKET_QUOTE);
  }
  auto book_quotes = ToBookQuotePair(quote);
  previous = quote;
  book_quotes.m_ask.m_mpid = mpid;
  update(book_quotes.m_ask, Type::MARKET_QUOTE);
  book_quotes.m_bid.m_mpid = mpid;
  update(book_quotes.m_bid, Type::MARKET_QUOTE);
}

void LocalBookViewModel::update_volume(Quantity volume) {
  m_volume = volume;
  m_volume_signal(m_volume);
}

void LocalBookViewModel::update_high(Money high) {
  m_high = high;
  m_high_signal(*m_high);
}

void LocalBookViewModel::update_low(Money low) {
  m_low = low;
  m_low_signal(*m_low);
}

void LocalBookViewModel::update_open(Money open) {
  m_open = open;
  m_open_signal(*m_open);
}

void LocalBookViewModel::update_close(Money close) {
  m_close = close;
  m_close_signal(*m_close);
}

void LocalBookViewModel::clear_book_quotes() {
  auto book_quotes = std::vector<BookQuote>();
  for(auto& bid : m_bids) {
    if(bid->m_type == Type::BOOK_QUOTE) {
      book_quotes.push_back(bid->m_quote);
    }
  }
  for(auto& ask : m_asks) {
    if(ask->m_type == Type::BOOK_QUOTE) {
      book_quotes.push_back(ask->m_quote);
    }
  }
  for(auto& book_quote : book_quotes) {
    book_quote.m_quote.m_size = 0;
    update(book_quote);
  }
}

void LocalBookViewModel::clear_market_quotes() {
  auto book_quotes = std::vector<BookQuote>();
  for(auto& bid : m_bids) {
    if(bid->m_type == Type::MARKET_QUOTE) {
      book_quotes.push_back(bid->m_quote);
    }
  }
  for(auto& ask : m_asks) {
    if(ask->m_type == Type::MARKET_QUOTE) {
      book_quotes.push_back(ask->m_quote);
    }
  }
  for(auto& book_quote : book_quotes) {
    book_quote.m_quote.m_size = 0;
    update(book_quote);
  }
}

const Security& LocalBookViewModel::get_security() const {
  return m_security;
}

const BboQuote& LocalBookViewModel::get_bbo() const {
  return m_bbo;
}

const std::vector<std::unique_ptr<BookViewModel::Quote>>&
    LocalBookViewModel::get_asks() const {
  return m_asks;
}

const std::vector<std::unique_ptr<BookViewModel::Quote>>&
    LocalBookViewModel::get_bids() const {
  return m_bids;
}

optional<Money> LocalBookViewModel::get_high() const {
  return m_high;
}

optional<Money> LocalBookViewModel::get_low() const {
  return m_low;
}

optional<Money> LocalBookViewModel::get_open() const {
  return m_open;
}

optional<Money> LocalBookViewModel::get_close() const {
  return m_close;
}

Quantity LocalBookViewModel::get_volume() const {
  return m_volume;
}

QtPromise<void> LocalBookViewModel::load() {
  return QtPromise();
}

connection LocalBookViewModel::connect_bbo_slot(
    const BboSignal::slot_type& slot) const {
  return m_bbo_signal.connect(slot);
}

connection LocalBookViewModel::connect_quote_slot(
    const QuoteSignal::slot_type& slot) const {
  return m_quote_signal.connect(slot);
}

connection LocalBookViewModel::connect_high_slot(
    const PriceSignal::slot_type& slot) const {
  return m_high_signal.connect(slot);
}

connection LocalBookViewModel::connect_low_slot(
    const PriceSignal::slot_type& slot) const {
  return m_low_signal.connect(slot);
}

connection LocalBookViewModel::connect_open_slot(
    const PriceSignal::slot_type& slot) const {
  return m_open_signal.connect(slot);
}

connection LocalBookViewModel::connect_close_slot(
    const PriceSignal::slot_type& slot) const {
  return m_close_signal.connect(slot);
}

connection LocalBookViewModel::connect_volume_slot(
    const QuantitySignal::slot_type& slot) const {
  return m_volume_signal.connect(slot);
}

void LocalBookViewModel::update(const BookQuote& quote, Type type) {
  auto& quotes = Pick(quote.m_quote.m_side, m_asks, m_bids);
  auto direction = GetDirection(quote.m_quote.m_side);
  auto lower_bound = [&] {
    for(auto i = quotes.rbegin(); i != quotes.rend(); ++i) {
      auto& book_quote = (*i)->m_quote;
      if(direction * book_quote.m_quote.m_price <=
          direction * quote.m_quote.m_price) {
        return i;
      }
    }
    return quotes.rend();
  }();
  auto existing_iterator = lower_bound;
  while(existing_iterator != quotes.rend() &&
      (*existing_iterator)->m_quote.m_quote.m_price == quote.m_quote.m_price &&
      (*existing_iterator)->m_quote.m_mpid != quote.m_mpid) {
    ++existing_iterator;
  }
  if(existing_iterator == quotes.rend() ||
      (*existing_iterator)->m_quote.m_quote.m_price != quote.m_quote.m_price) {
    if(quote.m_quote.m_size != 0) {
      auto insert_iterator = lower_bound;
      while(insert_iterator != quotes.rend() &&
          (*insert_iterator)->m_quote.m_quote.m_price ==
          quote.m_quote.m_price &&
          std::tie(quote.m_quote.m_size, quote.m_timestamp, quote.m_mpid) <
          std::tie((*insert_iterator)->m_quote.m_quote.m_size,
          (*insert_iterator)->m_quote.m_timestamp,
          (*insert_iterator)->m_quote.m_mpid)) {
        ++insert_iterator;
      }
      add(Quote{type, quote, 0},
        std::distance(quotes.rbegin(), insert_iterator));
    }
    return;
  }
  if(quote.m_quote.m_size == 0) {
    remove(quote.m_quote.m_side,
      std::distance(quotes.rbegin(), existing_iterator));
  } else {
    auto insert_iterator = lower_bound;
    while(insert_iterator != quotes.rend() &&
        (*insert_iterator)->m_quote.m_quote.m_price == quote.m_quote.m_price &&
        std::tie(quote.m_quote.m_size, quote.m_timestamp, quote.m_mpid) <
        std::tie((*insert_iterator)->m_quote.m_quote.m_size,
        (*insert_iterator)->m_quote.m_timestamp,
        (*insert_iterator)->m_quote.m_mpid)) {
      ++insert_iterator;
    }
    if(insert_iterator == existing_iterator) {
      (*insert_iterator)->m_quote.m_quote.m_size = quote.m_quote.m_size;
      (*insert_iterator)->m_quote.m_timestamp = quote.m_timestamp;
      auto quote_index = std::distance(quotes.rbegin(), insert_iterator);
      m_quote_signal(**insert_iterator, quote_index);
    } else {
      auto existing_index = std::distance(quotes.rbegin(), existing_iterator);
      auto quote_index = std::distance(quotes.rbegin(), insert_iterator);
      if(quote_index > existing_index) {
        --quote_index;
      }
      remove(quote.m_quote.m_side, existing_index);
      add(Quote{type, quote, 0}, quote_index);
    }
  }
}

void LocalBookViewModel::add(const Quote& quote, int index) {
  auto entry = std::make_unique<Quote>(quote);
  auto& quotes = Pick(quote.m_quote.m_quote.m_side, m_asks, m_bids);
  auto i = quotes.rbegin() + index;
  if(quotes.empty()) {
    quotes.push_back(std::move(entry));
    m_quote_signal(*quotes.back(), index);
  } else if(i != quotes.rend() &&
      quote.m_quote.m_quote.m_price == (*i)->m_quote.m_quote.m_price) {
    entry->m_price_level = (*i)->m_price_level;
    quotes.insert(i.base(), std::move(entry));
    m_quote_signal(*quotes[index], index);
  } else if(i != quotes.rbegin() &&
      quote.m_quote.m_quote.m_price == (*(i - 1))->m_quote.m_quote.m_price) {
    entry->m_price_level = (*(i - 1))->m_price_level;
    quotes.insert(i.base(), std::move(entry));
    m_quote_signal(*quotes[index], index);
  } else {
    if(i == quotes.rend()) {
      entry->m_price_level = (*(i - 1))->m_price_level + 1;
    } else {
      entry->m_price_level = (*i)->m_price_level;
    }
    for(auto j = i; j != quotes.rend(); ++j) {
      ++(*j)->m_price_level;
    }
    quotes.insert(i.base(), std::move(entry));
    m_quote_signal(*quotes[index], index);
  }
}

void LocalBookViewModel::remove(Side side, int index) {
  auto& quotes = Pick(side, m_asks, m_bids);
  auto i = quotes.rbegin() + index;
  auto quote = **i;
  if(i == (quotes.rend() - 1) ||
      (*i)->m_price_level == (*(i + 1))->m_price_level ||
      i != quotes.rbegin() &&
      (*i)->m_price_level == (*(i - 1))->m_price_level) {
    quotes.erase((i + 1).base());
    m_quote_signal(quote, index);
  } else {
    for(auto j = i + 1; j != quotes.rend(); ++j) {
      --(*j)->m_price_level;
    }
    quotes.erase((i + 1).base());
    m_quote_signal(quote, index);
  }
}
