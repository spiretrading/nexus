#include "Spire/BookView/ServicesBookViewModel.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/TechnicalAnalysis/StandardSecurityQueries.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::TechnicalAnalysis;
using namespace Spire;

ServicesBookViewModel::ServicesBookViewModel(Security security,
  Definitions definitions, ServiceClientsBox clients)
  : m_local_model(std::move(security), definitions),
    m_definitions(std::move(definitions)),
    m_clients(std::move(clients)),
    m_loader(std::make_shared<CallOnce<Mutex>>()) {}

const Security& ServicesBookViewModel::get_security() const {
  return m_local_model.get_security();
}

const BboQuote& ServicesBookViewModel::get_bbo() const {
  return m_local_model.get_bbo();
}

const std::vector<std::unique_ptr<BookViewModel::Quote>>&
    ServicesBookViewModel::get_asks() const {
  return m_local_model.get_asks();
}

const std::vector<std::unique_ptr<BookViewModel::Quote>>&
    ServicesBookViewModel::get_bids() const {
  return m_local_model.get_bids();
}

optional<Money> ServicesBookViewModel::get_high() const {
  return m_local_model.get_high();
}

optional<Money> ServicesBookViewModel::get_low() const {
  return m_local_model.get_low();
}

optional<Money> ServicesBookViewModel::get_open() const {
  return m_local_model.get_open();
}

optional<Money> ServicesBookViewModel::get_close() const {
  return m_local_model.get_close();
}

Quantity ServicesBookViewModel::get_volume() const {
  return m_local_model.get_volume();
}

QtPromise<void> ServicesBookViewModel::load() {
  auto security = get_security();
  auto bbo_slot = m_event_handler.get_slot<SequencedBboQuote>(
    [=] (const auto& bbo) { on_bbo(bbo); });
  auto book_quote_slot = m_event_handler.get_slot<BookQuote>(
    [=] (const auto& quote) { on_book_quote(quote); },
    [=] (const auto& e) { on_book_quote_interruption(e); });
  auto market_quote_slot = m_event_handler.get_slot<MarketQuote>(
    [=] (const auto& quote) { on_market_quote(quote); },
    [=] (const auto& e) { on_market_quote_interruption(e); });
  auto volume_slot = m_event_handler.get_slot<Nexus::Queries::QueryVariant>(
    [=] (const auto& value) { on_volume(value); });
  auto high_slot = m_event_handler.get_slot<Nexus::Queries::QueryVariant>(
    [=] (const auto& value) { on_high(value); });
  auto low_slot = m_event_handler.get_slot<Nexus::Queries::QueryVariant>(
    [=] (const auto& value) { on_low(value); });
  auto open_slot = m_event_handler.get_slot<TimeAndSale>(
    [=] (const auto& value) { on_open(value); });
  auto close_slot = m_event_handler.get_slot<TimeAndSale>(
    [=] (const auto& value) { on_close(value); });
  auto loader = m_loader;
  auto definitions = m_definitions;
  return QtPromise([=, clients = m_clients, bbo_slot = std::move(bbo_slot),
      book_quote_slot = std::move(book_quote_slot),
      market_quote_slot = std::move(market_quote_slot),
      volume_slot = std::move(volume_slot), high_slot = std::move(high_slot),
      low_slot = std::move(low_slot), open_slot = std::move(open_slot),
      close_slot = std::move(close_slot)] () mutable {
    loader->Call([&] {
      auto query = BuildCurrentQuery(security);
      query.SetInterruptionPolicy(InterruptionPolicy::IGNORE_CONTINUE);
      clients.GetMarketDataClient().QueryBboQuotes(query, std::move(bbo_slot));
      QueryRealTimeBookQuotesWithSnapshot(clients.GetMarketDataClient(),
        security, std::move(book_quote_slot), InterruptionPolicy::BREAK_QUERY);
      QueryRealTimeMarketQuotesWithSnapshot(clients.GetMarketDataClient(),
        security, std::move(market_quote_slot),
        InterruptionPolicy::BREAK_QUERY);
      QueryDailyVolume(clients.GetChartingClient(), security,
        clients.GetTimeClient().GetTime(), pos_infin,
        definitions.get_market_database(),
        definitions.get_time_zone_database(), std::move(volume_slot));
      QueryDailyHigh(clients.GetChartingClient(), security,
        clients.GetTimeClient().GetTime(), pos_infin,
        definitions.get_market_database(),
        definitions.get_time_zone_database(), std::move(high_slot));
      QueryDailyLow(clients.GetChartingClient(), security,
        clients.GetTimeClient().GetTime(), pos_infin,
        definitions.get_market_database(),
        definitions.get_time_zone_database(), std::move(low_slot));
      QueryOpen(clients.GetMarketDataClient(), security,
        clients.GetTimeClient().GetTime(), definitions.get_market_database(),
        definitions.get_time_zone_database(), "", std::move(open_slot));
      auto close = LoadPreviousClose(clients.GetMarketDataClient(), security,
        clients.GetTimeClient().GetTime(),
        definitions.get_market_database(),
        definitions.get_time_zone_database(), "");
      if(close) {
        close_slot.Push(*close);
      }
    });
  }, LaunchPolicy::ASYNC);
}

const MarketDatabase& ServicesBookViewModel::get_market_database() const {
  return m_definitions.get_market_database();
}

connection ServicesBookViewModel::connect_bbo_slot(
    const BboSignal::slot_type& slot) const {
  return m_local_model.connect_bbo_slot(slot);
}

connection ServicesBookViewModel::connect_quote_slot(
    const QuoteSignal::slot_type& slot) const {
  return m_local_model.connect_quote_slot(slot);
}

connection ServicesBookViewModel::connect_high_slot(
    const PriceSignal::slot_type& slot) const {
  return m_local_model.connect_high_slot(slot);
}

connection ServicesBookViewModel::connect_low_slot(
    const PriceSignal::slot_type& slot) const {
  return m_local_model.connect_low_slot(slot);
}

connection ServicesBookViewModel::connect_open_slot(
    const PriceSignal::slot_type& slot) const {
  return m_local_model.connect_open_slot(slot);
}

connection ServicesBookViewModel::connect_close_slot(
    const PriceSignal::slot_type& slot) const {
  return m_local_model.connect_close_slot(slot);
}

connection ServicesBookViewModel::connect_volume_slot(
    const QuantitySignal::slot_type& slot) const {
  return m_local_model.connect_volume_slot(slot);
}

void ServicesBookViewModel::on_bbo(const BboQuote& quote) {
  m_local_model.update(quote);
}

void ServicesBookViewModel::on_book_quote(const BookQuote& quote) {
  m_local_model.update(quote);
}

void ServicesBookViewModel::on_book_quote_interruption(
    const std::exception_ptr& e) {
  m_local_model.clear_book_quotes();
  QueryRealTimeBookQuotesWithSnapshot(m_clients.GetMarketDataClient(),
    m_local_model.get_security(), m_event_handler.get_slot<BookQuote>(
    [=] (const auto& quote) { on_book_quote(quote); },
    [=] (const auto& e) { on_book_quote_interruption(e); }),
    InterruptionPolicy::BREAK_QUERY);
}

void ServicesBookViewModel::on_market_quote(const MarketQuote& quote) {
  m_local_model.update(quote);
}

void ServicesBookViewModel::on_market_quote_interruption(
    const std::exception_ptr& e) {
  m_local_model.clear_market_quotes();
  QueryRealTimeMarketQuotesWithSnapshot(m_clients.GetMarketDataClient(),
    m_local_model.get_security(), m_event_handler.get_slot<MarketQuote>(
    [=] (const auto& quote) { on_market_quote(quote); },
    [=] (const auto& e) { on_market_quote_interruption(e); }),
    InterruptionPolicy::BREAK_QUERY);
}

void ServicesBookViewModel::on_volume(
    const Nexus::Queries::QueryVariant& value) {
  m_local_model.update_volume(get<Quantity>(value));
}

void ServicesBookViewModel::on_high(const Nexus::Queries::QueryVariant& value) {
  m_local_model.update_high(get<Money>(value));
}

void ServicesBookViewModel::on_low(const Nexus::Queries::QueryVariant& value) {
  m_local_model.update_low(get<Money>(value));
}

void ServicesBookViewModel::on_open(const TimeAndSale& value) {
  m_local_model.update_open(value.m_price);
}

void ServicesBookViewModel::on_close(const TimeAndSale& value) {
  m_local_model.update_close(value.m_price);
}
