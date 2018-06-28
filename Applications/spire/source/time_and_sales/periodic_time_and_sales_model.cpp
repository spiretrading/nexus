#include "spire/time_and_sales/periodic_time_and_sales_model.hpp"
#include <Beam/Threading/LiveTimer.hpp>

using namespace Beam;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

periodic_time_and_sales_model::periodic_time_and_sales_model(Security s,
      Beam::Threading::TimerThreadPool& timer_thread_pool)
    : m_security(std::move(s)),
      m_timer_thread_pool(&timer_thread_pool),
      m_price(Money::ONE),
      m_price_range(time_and_sales_properties::price_range::AT_ASK),
      m_period(pos_infin),
      m_load_duration(seconds(10)),
      m_volume(0) {
  connect(&m_timer, &QTimer::timeout, [=] {on_timeout();});
}

Money periodic_time_and_sales_model::get_price() const {
  return m_price;
}

void periodic_time_and_sales_model::set_price(Money price) {
  m_price = price;
}

time_and_sales_properties::price_range periodic_time_and_sales_model::
    get_price_range() const {
  return m_price_range;
}

void periodic_time_and_sales_model::set_price_range(
    time_and_sales_properties::price_range r) {
  m_price_range = r;
}

time_duration periodic_time_and_sales_model::get_load_duration() const {
  return m_load_duration;
}

void periodic_time_and_sales_model::set_load_duration(time_duration d) {
  m_load_duration = d;
}

time_duration periodic_time_and_sales_model::get_period() const {
  return m_period;
}

void periodic_time_and_sales_model::set_period(time_duration p) {
  m_period = p;
  m_timer.stop();
  if(m_period != pos_infin) {
    m_timer.start(m_period.total_milliseconds());
  }
}

const Security& periodic_time_and_sales_model::get_security() const {
  return m_security;
}

Quantity periodic_time_and_sales_model::get_volume() const {
  return m_volume;
}

qt_promise<std::vector<time_and_sales_model::entry>>
    periodic_time_and_sales_model::load_snapshot(Beam::Queries::Sequence last,
    int count) {
  auto insert = [&] (auto where, auto sequence) {
    auto timestamp = where->m_time_and_sale->m_timestamp - seconds(1);
    auto value = MakeSequencedValue(TimeAndSale(timestamp, m_price, 100,
      TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
      "NYSE"), sequence);
    return m_entries.insert(where, {value, m_price_range});
  };
  std::vector<time_and_sales_model::entry> snapshot;
  if(m_load_duration == pos_infin) {
    return make_qt_promise(
      [=] {
        return snapshot;
      });
  }
  if(m_entries.empty()) {
    auto value = MakeSequencedValue(TimeAndSale(second_clock::universal_time(),
      m_price, 100, TimeAndSale::Condition(
      TimeAndSale::Condition::Type::REGULAR, "@"), "NYSE"),
      Beam::Queries::Sequence(100000));
    m_entries.push_back({value, m_price_range});
  }
  auto i = std::lower_bound(m_entries.begin(), m_entries.end(), last,
    [] (auto& lhs, auto& rhs) {
      return lhs.m_time_and_sale.GetSequence() < rhs;
    });
  if(i == m_entries.end()) {
    --i;
  }
  auto e = Decrement(last);
  while(e.GetOrdinal() > count &&
      i->m_time_and_sale.GetSequence().GetOrdinal() !=
      (last.GetOrdinal() - count)) {
    if(i == m_entries.begin()) {
      i = insert(m_entries.begin(), e);
    } else {
      --i;
      if(i->m_time_and_sale.GetSequence() != e) {
        i = insert(i, e);
      }
    }
    e = Decrement(e);
  }
  snapshot.insert(snapshot.begin(), i, i + count);
  return make_qt_promise([d = m_load_duration, pool = m_timer_thread_pool,
      snapshot=std::move(snapshot)] {
    LiveTimer t(d, Ref(*pool));
    t.Start();
    t.Wait();
    return std::move(snapshot);
  });
}

connection periodic_time_and_sales_model::connect_time_and_sale_signal(
    const time_and_sale_signal::slot_type& slot) const {
  return m_time_and_sale_signal.connect(slot);
}

connection periodic_time_and_sales_model::connect_volume_signal(
    const volume_signal::slot_type& slot) const {
  return m_volume_signal.connect(slot);
}

void periodic_time_and_sales_model::on_timeout() {
  if(m_entries.empty()) {
    return;
  }
  auto sequence = Increment(m_entries.back().m_time_and_sale.GetSequence());
  auto value = MakeSequencedValue(TimeAndSale(second_clock::universal_time(),
    m_price, 100, TimeAndSale::Condition(
    TimeAndSale::Condition::Type::REGULAR, "@"), "NYSE"), sequence);
  m_entries.push_back({value, m_price_range});
  m_volume += value->m_size;
  m_time_and_sale_signal(m_entries.back());
  m_volume_signal(m_volume);
}
