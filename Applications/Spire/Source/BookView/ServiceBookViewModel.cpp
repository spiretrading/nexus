#include "Spire/BookView/ServiceBookViewModel.hpp"
#include <ranges>
#include "Nexus/Definitions/QuoteConversions.hpp"
#include "Spire/BookView/AggregateBookViewModel.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;

ServiceBookViewModel::ServiceBookViewModel(
    Security security, MarketDatabase markets, MarketDataClientBox client)
    : m_security(std::move(security)),
      m_markets(std::move(markets)),
      m_client(std::move(client)),
      m_model(make_local_aggregate_book_view_model()) {
  if(m_security == Security()) {
    return;
  }
  auto bbo_query = MakeCurrentQuery(m_security);
  bbo_query.SetInterruptionPolicy(InterruptionPolicy::IGNORE_CONTINUE);
  m_client.QueryBboQuotes(bbo_query, m_event_handler.get_slot<BboQuote>(
    std::bind_front(&ServiceBookViewModel::on_bbo, this)));
  QueryRealTimeBookQuotesWithSnapshot(m_client, m_security,
    m_event_handler.get_slot<BookQuote>(
      std::bind_front(&ServiceBookViewModel::on_book_quote, this),
      std::bind_front(&ServiceBookViewModel::on_book_quote_interruption, this)),
    InterruptionPolicy::BREAK_QUERY);
  QueryRealTimeMarketQuotesWithSnapshot(m_client, m_security,
    m_event_handler.get_slot<MarketQuote>(
      std::bind_front(&ServiceBookViewModel::on_market_quote, this),
      std::bind_front(
        &ServiceBookViewModel::on_market_quote_interruption, this)),
    InterruptionPolicy::BREAK_QUERY);
}

const std::shared_ptr<BookQuoteListModel>&
    ServiceBookViewModel::get_bids() const {
  return m_model->get_bids();
}

const std::shared_ptr<BookQuoteListModel>&
    ServiceBookViewModel::get_asks() const {
  return m_model->get_asks();
}

const std::shared_ptr<BookViewModel::UserOrderListModel>&
    ServiceBookViewModel::get_bid_orders() const {
  return m_model->get_bid_orders();
}

const std::shared_ptr<BookViewModel::UserOrderListModel>&
    ServiceBookViewModel::get_ask_orders() const {
  return m_model->get_ask_orders();
}

const std::shared_ptr<BookViewModel::PreviewOrderModel>&
    ServiceBookViewModel::get_preview_order() const {
  return m_model->get_preview_order();
}

const std::shared_ptr<BboQuoteModel>&
    ServiceBookViewModel::get_bbo_quote() const {
  return m_model->get_bbo_quote();
}

const std::shared_ptr<SecurityTechnicalsValueModel>&
    ServiceBookViewModel::get_technicals() const {
  return m_model->get_technicals();
}

void ServiceBookViewModel::on_bbo(const BboQuote& bbo) {
  m_model->get_bbo_quote()->set(bbo);
}

void ServiceBookViewModel::on_book_quote(const BookQuote& quote) {
  auto direction = GetDirection(quote.m_quote.m_side);
  auto quotes =
    Pick(quote.m_quote.m_side, m_model->get_asks(), m_model->get_bids());
  auto lower_bound = [&] {
    for(auto i = quotes->begin(); i != quotes->end(); ++i) {
      auto& book_quote = i->m_quote;
      if(direction * book_quote.m_price <= direction * quote.m_quote.m_price) {
        return i;
      }
    }
    return quotes->end();
  }();
  auto existing_iterator = lower_bound;
  while(existing_iterator != quotes->end() &&
      existing_iterator->m_quote.m_price == quote.m_quote.m_price &&
      existing_iterator->m_mpid != quote.m_mpid) {
    ++existing_iterator;
  }
  if(existing_iterator == quotes->end() ||
      existing_iterator->m_quote.m_price != quote.m_quote.m_price) {
    if(quote.m_quote.m_size != 0) {
      auto insert_iterator = lower_bound;
      while(insert_iterator != quotes->end() &&
          insert_iterator->m_quote.m_price == quote.m_quote.m_price &&
          std::tie(quote.m_quote.m_size, quote.m_timestamp, quote.m_mpid) <
          std::tie(insert_iterator->m_quote.m_size,
            insert_iterator->m_timestamp, insert_iterator->m_mpid)) {
        ++insert_iterator;
      }
      quotes->insert(quote, insert_iterator);
    }
    return;
  }
  if(quote.m_quote.m_size == 0) {
    quotes->remove(existing_iterator);
  } else {
    auto insert_iterator = lower_bound;
    while(insert_iterator != quotes->end() &&
        insert_iterator->m_quote.m_price == quote.m_quote.m_price &&
        std::tie(quote.m_quote.m_size, quote.m_timestamp, quote.m_mpid) <
        std::tie(insert_iterator->m_quote.m_size,
          insert_iterator->m_timestamp, insert_iterator->m_mpid)) {
      ++insert_iterator;
    }
    if(insert_iterator == existing_iterator) {
      *insert_iterator = quote;
    } else {
      quotes->remove(existing_iterator);
      quotes->insert(quote, insert_iterator);
    }
  }
}

void ServiceBookViewModel::on_market_quote(const MarketQuote& quote) {
  auto& previous_market_quote = m_market_quotes[quote.m_market];
  auto mpid = m_markets.FromCode(quote.m_market).m_displayName;
  if(!previous_market_quote.m_market.IsEmpty()) {
    auto previous_book_quotes = ToBookQuotePair(previous_market_quote);
    previous_book_quotes.m_ask.m_quote.m_size = 0;
    previous_book_quotes.m_ask.m_mpid = mpid;
    previous_book_quotes.m_bid.m_quote.m_size = 0;
    previous_book_quotes.m_bid.m_mpid = mpid;
    on_book_quote(previous_book_quotes.m_ask);
    on_book_quote(previous_book_quotes.m_bid);
  }
  auto book_quotes = ToBookQuotePair(quote);
  previous_market_quote = quote;
  book_quotes.m_ask.m_mpid = mpid;
  book_quotes.m_bid.m_mpid = mpid;
  on_book_quote(book_quotes.m_ask);
  on_book_quote(book_quotes.m_bid);
}

void ServiceBookViewModel::on_book_quote_interruption(
    const std::exception_ptr&) {
}

void ServiceBookViewModel::on_market_quote_interruption(
    const std::exception_ptr&) {
  auto market_quotes = m_market_quotes;
  for(auto& market_quote : market_quotes | std::views::values) {
    auto clear_quote = market_quote;
    clear_quote.m_ask.m_size = 0;
    clear_quote.m_bid.m_size = 0;
    on_market_quote(clear_quote);
  }
  QueryRealTimeMarketQuotesWithSnapshot(m_client, m_security,
    m_event_handler.get_slot<MarketQuote>(
      std::bind_front(&ServiceBookViewModel::on_market_quote, this),
      std::bind_front(
        &ServiceBookViewModel::on_market_quote_interruption, this)),
    InterruptionPolicy::BREAK_QUERY);
}
