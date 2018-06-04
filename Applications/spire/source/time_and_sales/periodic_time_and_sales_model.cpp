#include "spire/time_and_sales/periodic_time_and_sales_model.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

periodic_time_and_sales_model::periodic_time_and_sales_model(Security s)
    : m_security(std::move(s)),
      m_price(Money::ONE),
      m_price_range(time_and_sales_properties::price_range::AT_ASK),
      m_period(pos_infin),
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
  std::vector<time_and_sales_model::entry> snapshot;
  if(m_entries.empty()) {
    return make_qt_promise([snapshot=std::move(snapshot)] {
      return std::move(snapshot);
    });
  }
  last = std::min(last, m_entries.back().m_time_and_sale.GetSequence());
  auto front = m_entries.front().m_time_and_sale.GetSequence();
  if(last < front) {
    return make_qt_promise([snapshot=std::move(snapshot)] {
      return std::move(snapshot);
    });
  }
  int first = std::max(static_cast<int>(front.GetOrdinal()),
    static_cast<int>(last.GetOrdinal()) - count);
  while(count != 0 && first <= last.GetOrdinal()) {
    snapshot.push_back(m_entries[first]);
    ++first;
  }
  return make_qt_promise([snapshot=std::move(snapshot)] {
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
  auto sequence = [&] {
    if(m_entries.empty()) {
      return Beam::Queries::Sequence::First();
    }
    return Increment(m_entries.back().m_time_and_sale.GetSequence());
  }();
  auto value = MakeSequencedValue(TimeAndSale(second_clock::universal_time(),
    m_price, 100, TimeAndSale::Condition(
    TimeAndSale::Condition::Type::REGULAR, "@"), "NYSE"), sequence);
  m_entries.push_back({value, m_price_range});
  m_volume += value->m_size;
  m_time_and_sale_signal(m_entries.back());
  m_volume_signal(m_volume);
}
