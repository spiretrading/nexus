#include "Spire/TimeAndSalesUiTester/PeriodicTimeAndSalesModel.hpp"
#include <Beam/Threading/LiveTimer.hpp>

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

PeriodicTimeAndSalesModel::PeriodicTimeAndSalesModel(Security security)
    : m_security(std::move(security)),
      m_price(Money::ONE),
      m_price_range(TimeAndSalesProperties::PriceRange::AT_ASK),
      m_period(pos_infin),
      m_load_duration(seconds(10)),
      m_volume(0),
      m_is_loaded(std::make_shared<std::atomic_bool>(false)) {
  connect(&m_timer, &QTimer::timeout, [=] { on_timeout(); });
}

Money PeriodicTimeAndSalesModel::get_price() const {
  return m_price;
}

void PeriodicTimeAndSalesModel::set_price(Money price) {
  m_price = price;
}

TimeAndSalesProperties::PriceRange PeriodicTimeAndSalesModel::
    get_price_range() const {
  return m_price_range;
}

void PeriodicTimeAndSalesModel::set_price_range(
    TimeAndSalesProperties::PriceRange r) {
  m_price_range = r;
}

time_duration PeriodicTimeAndSalesModel::get_load_duration() const {
  return m_load_duration;
}

void PeriodicTimeAndSalesModel::set_load_duration(time_duration d) {
  m_load_duration = d;
}

time_duration PeriodicTimeAndSalesModel::get_period() const {
  return m_period;
}

void PeriodicTimeAndSalesModel::set_period(time_duration p) {
  m_period = p;
  m_timer.stop();
  if(m_period != pos_infin) {
    m_timer.start(static_cast<int>(m_period.total_milliseconds()));
  }
}

const Security& PeriodicTimeAndSalesModel::get_security() const {
  return m_security;
}

Quantity PeriodicTimeAndSalesModel::get_volume() const {
  return m_volume;
}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    PeriodicTimeAndSalesModel::load_snapshot(Beam::Queries::Sequence last,
    int count) {
  auto insert = [&] (auto where, auto sequence) {
    auto timestamp = where->m_time_and_sale->m_timestamp - seconds(1);
    auto value = SequencedValue(TimeAndSale(timestamp, m_price, 100,
      TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
      "NYSE"), sequence);
    return m_entries.insert(where, {value, m_price_range});
  };
  auto snapshot = std::vector<TimeAndSalesModel::Entry>();
  if(m_load_duration == pos_infin) {
    return QtPromise(
      [=] {
        return snapshot;
      });
  }
  if(m_entries.empty()) {
    auto value = SequencedValue(TimeAndSale(second_clock::universal_time(),
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
  return QtPromise([duration = m_load_duration, snapshot = std::move(snapshot),
      is_loaded = m_is_loaded] {
    auto timer = LiveTimer(duration);
    timer.Start();
    timer.Wait();
    *is_loaded = true;
    return std::move(snapshot);
  }, LaunchPolicy::ASYNC);
}

connection PeriodicTimeAndSalesModel::connect_time_and_sale_signal(
    const TimeAndSaleSignal::slot_type& slot) const {
  return m_time_and_sale_signal.connect(slot);
}

connection PeriodicTimeAndSalesModel::connect_volume_signal(
    const VolumeSignal::slot_type& slot) const {
  return m_volume_signal.connect(slot);
}

void PeriodicTimeAndSalesModel::on_timeout() {
  if(!*m_is_loaded) {
    return;
  }
  auto sequence = Increment(m_entries.back().m_time_and_sale.GetSequence());
  auto value = SequencedValue(TimeAndSale(second_clock::universal_time(),
    m_price, 100, TimeAndSale::Condition(
    TimeAndSale::Condition::Type::REGULAR, "@"), "NYSE"), sequence);
  m_entries.push_back({value, m_price_range});
  m_volume += value->m_size;
  m_time_and_sale_signal(m_entries.back());
  m_volume_signal(m_volume);
}
