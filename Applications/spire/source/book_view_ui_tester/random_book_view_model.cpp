#include "spire/book_view_ui_tester/random_book_view_model.hpp"
#include <Beam/Threading/LiveTimer.hpp>

using namespace Beam;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

RandomBookViewModel::RandomBookViewModel(Security security,
    time_duration load_time, TimerThreadPool& timer_thread_pool)
    : m_security(std::move(security)),
      m_load_time(load_time),
      m_timer_thread_pool(&timer_thread_pool),
      m_period(seconds(1)),
      m_bbo(Quote(100 * Money::ONE, 1000, Side::BID),
        Quote(100 * Money::ONE + Money::CENT, 1000, Side::ASK),
        second_clock::universal_time()),
      m_random_engine(std::random_device()()) {
  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, [=] { on_timeout(); });
}

time_duration RandomBookViewModel::get_period() const {
  return m_period;
}

void RandomBookViewModel::set_period(time_duration period) {
  m_period = period;
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
  auto period = m_period;
  return make_qt_promise([=] {
    m_loading_flag.Call(
      [&] {
        LiveTimer load_timer(m_load_time, Ref(*m_timer_thread_pool));
        load_timer.Start();
        load_timer.Wait();
        for(auto i = 0; i < 1000; ++i) {
          update();
        }
        m_timer.start(period.total_milliseconds());
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

void RandomBookViewModel::update() {}

void RandomBookViewModel::on_timeout() {
  update();
  m_timer.start(m_period.total_milliseconds());
}
