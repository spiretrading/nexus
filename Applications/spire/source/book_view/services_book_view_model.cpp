#include "spire/book_view/services_book_view_model.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;

ServicesBookViewModel::ServicesBookViewModel(Security security,
    Definitions definitions, Ref<VirtualServiceClients> clients)
    : m_local_model(std::move(security)),
      m_definitions(std::move(definitions)),
      m_clients(clients.Get()) {
  auto query = BuildCurrentQuery(get_security());
  query.SetInterruptionPolicy(InterruptionPolicy::IGNORE_CONTINUE);
  m_clients->GetMarketDataClient().QueryBboQuotes(query,
    m_event_handler.get_slot<SequencedBboQuote>(
    [=] (const auto& bbo) { on_bbo(bbo); }));
  QueryRealTimeBookQuotesWithSnapshot(m_clients->GetMarketDataClient(),
    get_security(), m_event_handler.get_slot<BookQuote>(
    [=] (const auto& quote) { on_book_quote(quote); },
    [=] (const auto& e) { on_book_quote_interruption(e); }),
    InterruptionPolicy::BREAK_QUERY);
  QueryRealTimeMarketQuotesWithSnapshot(m_clients->GetMarketDataClient(),
    get_security(), m_event_handler.get_slot<MarketQuote>(
    [=] (const auto& quote) { on_market_quote(quote); },
    [=] (const auto& e) { on_market_quote_interruption(e); }),
    InterruptionPolicy::BREAK_QUERY);
}

const Security& ServicesBookViewModel::get_security() const {
  return m_local_model.get_security();
}

const BboQuote& ServicesBookViewModel::get_bbo() const {
  return m_local_model.get_bbo();
}

const std::vector<BookQuote>& ServicesBookViewModel::get_asks() const {
  return m_local_model.get_asks();
}

const std::vector<BookQuote>& ServicesBookViewModel::get_bids() const {
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
  return make_qt_promise([] {});
}

connection ServicesBookViewModel::connect_bbo_slot(
    const BboSignal::slot_type& slot) const {
  return m_local_model.connect_bbo_slot(slot);
}

connection ServicesBookViewModel::connect_book_quote_slot(
    const BookQuoteSignal::slot_type& slot) const {
  return m_local_model.connect_book_quote_slot(slot);
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
}

void ServicesBookViewModel::on_market_quote(const MarketQuote& quote) {
}

void ServicesBookViewModel::on_market_quote_interruption(
    const std::exception_ptr& e) {
}
