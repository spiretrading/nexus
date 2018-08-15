#include "spire/book_view_ui_tester/random_book_view_model.hpp"
#include <Beam/Threading/LiveTimer.hpp>

using namespace Beam;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

RandomBookViewModel::RandomBookViewModel(Security security,
    time_duration load_time, TimerThreadPool& timer_thread_pool)
    : m_security(std::move(security)),
      m_load_time(load_time),
      m_timer_thread_pool(&timer_thread_pool),
      m_is_loaded(false),
      m_bbo(Quote(100 * Money::ONE, 100, Side::BID),
        Quote(100 * Money::ONE + Money::CENT, 100, Side::ASK),
        second_clock::universal_time()),
      m_random_engine(std::random_device()()),
      m_loading_flag(std::make_shared<CallOnce<Mutex>>()),
      m_received_first_quote(false),
      m_received_second_quote(false) {
  connect(&m_timer, &QTimer::timeout, [=] { on_timeout(); });
  set_period(seconds(1));
}

RandomBookViewModel::~RandomBookViewModel() {
  m_loading_flag->Call(
    [&] {
      m_is_loaded = true;
    });
}

time_duration RandomBookViewModel::get_period() const {
  return m_period;
}

void RandomBookViewModel::set_period(time_duration period) {
  m_period = period;
  m_timer.stop();
  if(m_period != pos_infin) {
    m_timer.start(m_period.total_milliseconds());
  }
}

const Security& RandomBookViewModel::get_security() const {
  return m_security;
}

const BboQuote& RandomBookViewModel::get_bbo() const {
  return m_bbo;
}

const std::vector<BookQuote>& RandomBookViewModel::get_asks() const {
  return m_asks;
}

const std::vector<BookQuote>& RandomBookViewModel::get_bids() const {
  return m_bids;
}

optional<Money> RandomBookViewModel::get_high() const {
  return m_high;
}

optional<Money> RandomBookViewModel::get_low() const {
  return m_low;
}

optional<Money> RandomBookViewModel::get_open() const {
  return m_open;
}

optional<Money> RandomBookViewModel::get_close() const {
  return m_close;
}

Quantity RandomBookViewModel::get_volume() const {
  return m_volume;
}

QtPromise<void> RandomBookViewModel::load() {
  auto load_time = m_load_time;
  auto timer_thread_pool = m_timer_thread_pool;
  auto loading_flag = m_loading_flag;
  return make_qt_promise([=] {
    LiveTimer load_timer(load_time, Ref(*timer_thread_pool));
    load_timer.Start();
    load_timer.Wait();
    loading_flag->Call(
      [&] {
        for(auto i = 0; i < 1000; ++i) {
          update();
        }
        m_is_loaded = true;
      });
  });
}

connection RandomBookViewModel::connect_bbo_slot(
    const BboSignal::slot_type& slot) const {
  return m_bbo_signal.connect(slot);
}

connection RandomBookViewModel::connect_book_quote_slot(
    const BookQuoteSignal::slot_type& slot) const {
  return m_book_quote_signal.connect(slot);
}

connection RandomBookViewModel::connect_high_slot(
    const PriceSignal::slot_type& slot) const {
  return m_high_signal.connect(slot);
}

connection RandomBookViewModel::connect_low_slot(
    const PriceSignal::slot_type& slot) const {
  return m_low_signal.connect(slot);
}

connection RandomBookViewModel::connect_open_slot(
    const PriceSignal::slot_type& slot) const {
  return m_open_signal.connect(slot);
}

connection RandomBookViewModel::connect_close_slot(
    const PriceSignal::slot_type& slot) const {
  return m_close_signal.connect(slot);
}

connection RandomBookViewModel::connect_volume_slot(
    const QuantitySignal::slot_type& slot) const {
  return m_volume_signal.connect(slot);
}

void RandomBookViewModel::update() {
  update_bbo();
  update_time_and_sales();
}

void RandomBookViewModel::update_bbo() {
  auto random_num = m_random_engine() % 3;
  if(random_num == 0) {
    return;
  }
  auto& bid_price = m_bbo.m_bid.m_price;
  auto& ask_price = m_bbo.m_ask.m_price;
  if(random_num == 1) {
    if(bid_price > Money::CENT) {
      bid_price -= Money::CENT;
      ask_price -= Money::CENT;
      m_bbo_signal(m_bbo);
    }
  } else if(random_num == 2) {
    bid_price += Money::CENT;
    ask_price += Money::CENT;
    m_bbo_signal(m_bbo);
  }
}

void RandomBookViewModel::update_time_and_sales() {
  auto random_num = m_random_engine() % 3;
  if(random_num == 0) {
    return;
  }
  Money quote;
  if(random_num == 1) {
    quote = m_bbo.m_bid.m_price;
  } else if(random_num == 2) {
    quote = m_bbo.m_ask.m_price;
  }
  if(!m_received_first_quote) {
    m_received_first_quote = true;
    m_close = quote;
    m_close_signal(quote);
  } else if(!m_received_second_quote) {
    m_received_second_quote = true;
    m_open = quote;
    m_high = quote;
    m_low = quote;
    m_open_signal(m_open);
    m_high_signal(m_high);
    m_low_signal(m_low);
  }
  if(quote < m_low) {
    m_low = quote;
    m_low_signal(m_low);
  } else if(quote > m_high) {
    m_high = quote;
    m_high_signal(m_high);
  }
  m_volume += 100;
  m_volume_signal(m_volume);
}

void RandomBookViewModel::on_timeout() {
  if(!m_is_loaded) {
    return;
  }
  update();
  m_timer.start(m_period.total_milliseconds());
}
