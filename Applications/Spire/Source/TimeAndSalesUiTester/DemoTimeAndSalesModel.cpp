#include "Spire/TimeAndSalesUiTester/DemoTimeAndSalesModel.hpp"
#include <QRandomGenerator>
#include <Beam/Threading/LiveTimer.hpp>

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto markets = std::vector<std::string>{"XNYS", "TSE", "CHD", "CHI"};

  ptime from_time_t_milliseconds(std::time_t t) {
    return ptime(gregorian::date(1970, 1, 1)) + milliseconds(t);
  }

  std::time_t to_time_t_milliseconds(ptime pt) {
    return (pt - ptime(gregorian::date(1970, 1, 1))).total_milliseconds();
  }

  auto make_time_and_sale(ptime timestamp, Money price, Quantity size,
      const std::string& market) {
    return TimeAndSale(timestamp, price, size, TimeAndSale::Condition(
      TimeAndSale::Condition::Type::REGULAR, "@"), market);
  }
}

DemoTimeAndSalesModel::DemoTimeAndSalesModel(Security security)
    : m_security(std::move(security)),
      m_price(Money::ONE),
      m_indicator(BboIndicator::UNKNOWN),
      m_period(seconds(1)),
      m_query_duration(seconds(5)),
      m_is_data_random(false) {
  QObject::connect(&m_timer, &QTimer::timeout,
    std::bind_front(&DemoTimeAndSalesModel::on_timeout, this));
}

Money DemoTimeAndSalesModel::get_price() const {
  return m_price;
}

void DemoTimeAndSalesModel::set_price(Money price) {
  m_price = price;
}

BboIndicator DemoTimeAndSalesModel::get_bbo_indicator() const {
  return m_indicator;
}

void DemoTimeAndSalesModel::set_bbo_indicator(BboIndicator indicator) {
  m_indicator = indicator;
}

time_duration DemoTimeAndSalesModel::get_period() const {
  return m_period;
}

void DemoTimeAndSalesModel::set_period(time_duration period) {
  m_period = period;
  m_timer.stop();
  if(m_period != pos_infin) {
    m_timer.start(static_cast<int>(m_period.total_milliseconds()));
  }
}

time_duration DemoTimeAndSalesModel::get_query_duration() const {
  return m_query_duration;
}

void DemoTimeAndSalesModel::set_query_duration(time_duration duration) {
  m_query_duration = duration;
}

bool DemoTimeAndSalesModel::is_data_random() const {
  return m_is_data_random;
}

void DemoTimeAndSalesModel::set_data_random(bool is_random) {
  m_is_data_random = is_random;
}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
DemoTimeAndSalesModel::query_until(Queries::Sequence sequence, int max_count) {
  return QtPromise([=] {
    auto result = std::vector<TimeAndSalesModel::Entry>();
    if(m_query_duration == pos_infin) {
      return result;
    }
    auto populate = [&] (ptime timestamp) {
      auto count = max_count;
      while(count > 0) {
        result.insert(std::begin(result), make_entry(timestamp));
        --count;
        timestamp -= m_period;
      }
    };
    auto now = microsec_clock::universal_time();
    if(sequence >= Queries::Sequence(to_time_t_milliseconds(now))) {
      populate(now);
    } else {
      populate(from_time_t_milliseconds(sequence.GetOrdinal()) - m_period);
    }
    auto timer = LiveTimer(m_query_duration);
    timer.Start();
    timer.Wait();
    return result;
  }, LaunchPolicy::ASYNC);
}

connection DemoTimeAndSalesModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

DemoTimeAndSalesModel::Entry DemoTimeAndSalesModel::make_entry(
    ptime timestamp) const {
  if(m_is_data_random) {
    auto random_generator = QRandomGenerator(to_time_t_milliseconds(timestamp));
    return {SequencedValue(make_time_and_sale(timestamp,
        Truncate(Money(random_generator.bounded(2000.0)), 2),
        random_generator.bounded(1, 10000),
        markets[random_generator.bounded(markets.size())]),
      Queries::Sequence(to_time_t_milliseconds(timestamp))),
      static_cast<BboIndicator>(random_generator.bounded(6))};
  }
  return {SequencedValue(
    make_time_and_sale(timestamp, m_price, 100, markets.front()),
    Queries::Sequence(to_time_t_milliseconds(timestamp))), m_indicator};
}

void DemoTimeAndSalesModel::on_timeout() {
  m_update_signal(make_entry(microsec_clock::universal_time()));
}
