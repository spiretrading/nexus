#include "Spire/TimeAndSales/LocalTimeANdSalesModel.hpp"
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
  ptime from_time_t_milliseconds(std::time_t t) {
    return ptime(gregorian::date(1970,1,1)) + milliseconds(t);
  }

  std::time_t to_time_t_milliseconds(ptime pt) {
    return (pt - ptime(gregorian::date(1970,1,1))).total_milliseconds();
  }

  auto make_time_and_sale(ptime timestamp, Money price) {
    return TimeAndSale(timestamp, price, 100, TimeAndSale::Condition(
      TimeAndSale::Condition::Type::REGULAR, "@"), "NYSE");
  }
}

LocalTimeAndSalesModel::LocalTimeAndSalesModel(Security security) 
    : m_security(std::move(security)),
      m_price(Money::ONE),
      m_indicator(BboIndicator::UNKNOWN),
      m_period(seconds(1)),
      m_query_duration(seconds(1)) {
  QObject::connect(&m_timer, &QTimer::timeout,
    std::bind_front(&LocalTimeAndSalesModel::on_timeout, this));
}

const Security& LocalTimeAndSalesModel::get_security() const {
  return m_security;
}

Money LocalTimeAndSalesModel::get_price() const {
  return m_price;
}

void LocalTimeAndSalesModel::set_price(Money price) {
  m_price = price;
}

BboIndicator LocalTimeAndSalesModel::get_bbo_indicator() const {
  return m_indicator;
}

void LocalTimeAndSalesModel::set_bbo_indicator(BboIndicator indicator) {
  m_indicator = indicator;
}

time_duration LocalTimeAndSalesModel::get_period() const {
  return m_period;
}

void LocalTimeAndSalesModel::set_period(time_duration period) {
  m_period = period;
  m_timer.stop();
  if(m_period != pos_infin) {
    m_timer.start(static_cast<int>(m_period.total_milliseconds()));
  }
}

time_duration LocalTimeAndSalesModel::get_query_duration() const {
  return m_query_duration;
}

void LocalTimeAndSalesModel::set_query_duration(time_duration duration) {
  m_query_duration = duration;
}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    LocalTimeAndSalesModel::query_until(Queries::Sequence sequence,
      int max_count) {
  return QtPromise([=] {
    auto result = std::vector<TimeAndSalesModel::Entry>();
    auto populate = [&] (ptime timestamp) {
      auto count = max_count;
      while(count > 0) {
        result.insert(std::begin(result),
          {SequencedValue(make_time_and_sale(timestamp, m_price),
            Queries::Sequence(to_time_t_milliseconds(timestamp))),
          m_indicator});
        --count;
        timestamp -= m_period;
      }
    };
    auto now = microsec_clock::local_time();
    if(sequence >= Queries::Sequence(to_time_t_milliseconds(now))) {
      populate(now);
    } else {
      populate(from_time_t_milliseconds(sequence.GetOrdinal()));
    }
    auto timer = LiveTimer(m_query_duration);
    timer.Start();
    timer.Wait();
    return result;
  }, LaunchPolicy::ASYNC);
}

connection LocalTimeAndSalesModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

void LocalTimeAndSalesModel::on_timeout() {
  auto timestamp = microsec_clock::local_time();
  m_update_signal({SequencedValue(make_time_and_sale(timestamp, m_price),
    Queries::Sequence(to_time_t_milliseconds(timestamp))), m_indicator});
}
