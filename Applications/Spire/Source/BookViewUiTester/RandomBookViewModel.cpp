#include "Spire/BookViewUiTester/RandomBookViewModel.hpp"
#include <Beam/Threading/LiveTimer.hpp>

using namespace Beam;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

RandomBookViewModel::RandomBookViewModel(Security security,
    Definitions definitions, time_duration load_time)
    : m_model(std::move(security), std::move(definitions)),
      m_load_time(load_time),
      m_random_engine(std::random_device()()),
      m_loader(std::make_shared<CallOnce<Mutex>>()) {
  connect(&m_timer, &QTimer::timeout, this, &RandomBookViewModel::on_timeout);
  set_period(seconds(1));
}

time_duration RandomBookViewModel::get_period() const {
  return m_period;
}

void RandomBookViewModel::set_period(time_duration period) {
  m_period = period;
  m_timer.stop();
  if(m_period != pos_infin) {
    m_timer.start(static_cast<int>(m_period.total_milliseconds()));
  }
}

void RandomBookViewModel::publish(const BookQuote& quote) {
  m_model.update(quote);
}

const Security& RandomBookViewModel::get_security() const {
  return m_model.get_security();
}

const BboQuote& RandomBookViewModel::get_bbo() const {
  return m_model.get_bbo();
}

const std::vector<std::unique_ptr<BookViewModel::Quote>>&
    RandomBookViewModel::get_asks() const {
  return m_model.get_asks();
}

const std::vector<std::unique_ptr<BookViewModel::Quote>>&
    RandomBookViewModel::get_bids() const {
  return m_model.get_bids();
}

optional<Money> RandomBookViewModel::get_high() const {
  return m_model.get_high();
}

optional<Money> RandomBookViewModel::get_low() const {
  return m_model.get_low();
}

optional<Money> RandomBookViewModel::get_open() const {
  return m_model.get_open();
}

optional<Money> RandomBookViewModel::get_close() const {
  return m_model.get_close();
}

Quantity RandomBookViewModel::get_volume() const {
  return m_model.get_volume();
}

QtPromise<void> RandomBookViewModel::load() {
  auto load_time = m_load_time;
  auto loader = m_loader;
  return QtPromise([=] {
    auto load_timer = LiveTimer(load_time);
    load_timer.Start();
    load_timer.Wait();
    loader->Call([&] {
      for(auto i = 0; i < 1000; ++i) {
        update();
      }
      auto bid_quote = Nexus::Quote(Money(), 100, Side::BID);
      auto ask_quote = Nexus::Quote(Money(), 100, Side::ASK);
      for(auto i = 0; i < 100; ++i) {
        bid_quote.m_price = (100 * Money::ONE) - (i * Money::CENT);
        auto market = get_random_market();
        m_model.update(BookQuote(market.GetData(), true, market, bid_quote,
          second_clock::universal_time()));
        market = get_random_market();
        ask_quote.m_price = (100 * Money::ONE) + (i * Money::CENT);
        m_model.update(BookQuote(market.GetData(), true, market, ask_quote,
          second_clock::universal_time()));
      }
    });
  }, LaunchPolicy::ASYNC);
}

const MarketDatabase& RandomBookViewModel::get_market_database() const {
  return m_model.get_market_database();
}

connection RandomBookViewModel::connect_bbo_slot(
    const BboSignal::slot_type& slot) const {
  return m_model.connect_bbo_slot(slot);
}

connection RandomBookViewModel::connect_quote_slot(
    const QuoteSignal::slot_type& slot) const {
  return m_model.connect_quote_slot(slot);
}

connection RandomBookViewModel::connect_high_slot(
    const PriceSignal::slot_type& slot) const {
  return m_model.connect_high_slot(slot);
}

connection RandomBookViewModel::connect_low_slot(
    const PriceSignal::slot_type& slot) const {
  return m_model.connect_low_slot(slot);
}

connection RandomBookViewModel::connect_open_slot(
    const PriceSignal::slot_type& slot) const {
  return m_model.connect_open_slot(slot);
}

connection RandomBookViewModel::connect_close_slot(
    const PriceSignal::slot_type& slot) const {
  return m_model.connect_close_slot(slot);
}

connection RandomBookViewModel::connect_volume_slot(
    const QuantitySignal::slot_type& slot) const {
  return m_model.connect_volume_slot(slot);
}

MarketCode RandomBookViewModel::get_random_market() {
  auto& markets = m_model.get_definitions().get_market_database().GetEntries();
  auto index = m_random_engine() % markets.size();
  return markets[index].m_code;
}

void RandomBookViewModel::update() {
  update_bbo();
  update_book_quote();
  update_time_and_sales();
}

void RandomBookViewModel::update_bbo() {
  auto random_num = m_random_engine() % 3;
  if(random_num == 0) {
    return;
  }
  auto current_bbo = m_model.get_bbo();
  auto& bid_price = current_bbo.m_bid.m_price;
  auto& ask_price = current_bbo.m_ask.m_price;
  bid_price = m_random_engine() % 200000 * Money::ONE +
    m_random_engine() % 100 * Money::CENT;
  current_bbo.m_bid.m_size = m_random_engine() % 200000;
  ask_price = (m_random_engine() % 200) * Money::ONE +
    m_random_engine() % 100 * Money::CENT;
  current_bbo.m_ask.m_size = m_random_engine() % 100000;
  m_model.update(current_bbo);
}

void RandomBookViewModel::update_book_quote() {
  if(m_model.get_bids().size() == 0 || m_model.get_asks().size() == 0) {
    return;
  }
  auto side = [&] {
    if(m_random_engine() % 2 == 0) {
      return Side::BID;
    }
    return Side::ASK;
  }();
  auto& quotes = Pick(side, m_model.get_asks(), m_model.get_bids());
  auto& random_quote = *quotes[m_random_engine() % quotes.size()];
  auto book_quote = BookQuote();
  auto random_num = m_random_engine() % 100;
  if(random_num < 10) {
    book_quote = random_quote.m_quote;
    book_quote.m_quote.m_size = (m_random_engine() % 200) + 1;
  } else if(random_num >= 10 && random_num < 45) {
    auto market = get_random_market();
    auto quote = Nexus::Quote((m_random_engine() % 200) * Money::ONE, 100,
      side);
    book_quote = BookQuote(market.GetData(), true, market, quote,
      second_clock::universal_time());
  } else {
    book_quote = random_quote.m_quote;
    book_quote.m_quote.m_size = 0;
  }
  publish(book_quote);
}

void RandomBookViewModel::update_time_and_sales() {
  auto random_num = m_random_engine() % 3;
  if(random_num == 0) {
    return;
  }
  auto quote = [&] {
    if(random_num == 1) {
      return m_model.get_bbo().m_bid.m_price;
    } else {
      return m_model.get_bbo().m_ask.m_price;
    }
  }();
  if(!m_model.get_open().is_initialized()) {
    m_model.update_open(quote);
    m_model.update_close(quote);
    m_model.update_high(quote);
    m_model.update_low(quote);
  }
  if(quote < *m_model.get_low()) {
    m_model.update_low(quote);
  }
  if(quote > *m_model.get_high()) {
    m_model.update_high(quote);
  }
  m_model.update_volume(m_model.get_volume() + 100);
}

void RandomBookViewModel::on_timeout() {
  if(m_model.get_bbo().m_ask.m_price == Money::ZERO) {
    return;
  }
  update();
  m_timer.start(static_cast<int>(m_period.total_milliseconds()));
}
