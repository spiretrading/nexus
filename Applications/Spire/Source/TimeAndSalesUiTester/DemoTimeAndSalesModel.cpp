#include "Spire/TimeAndSalesUiTester/DemoTimeAndSalesModel.hpp"
#include <QRandomGenerator>

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto markets = std::vector<std::string>{"XNYS", "TSE", "CHD", "CHI"};
  const auto mpids = std::vector<std::string>{"NBFI", "CIBC", "PFSS", "RBCD"};

  std::time_t to_time_t_milliseconds(ptime pt) {
    return (pt - time_from_string("1970-01-01 00:00:00")).total_milliseconds();
  }

  auto make_time_and_sale(ptime timestamp, Money price, Quantity size,
      const std::string& market, const std::string& buyer,
      const std::string& seller) {
    return TimeAndSale(timestamp, price, size, TimeAndSale::Condition(
      TimeAndSale::Condition::Type::REGULAR, "@"), market, buyer, seller);
  }
}

DemoTimeAndSalesModel::DemoTimeAndSalesModel()
    : m_price(Money::ONE),
      m_indicator(BboIndicator::UNKNOWN),
      m_period(seconds(1)),
      m_query_duration(millisec(100)),
      m_resume_update_time(neg_infin),
      m_is_data_random(false) {
  QObject::connect(&m_timer, &QTimer::timeout,
    std::bind_front(&DemoTimeAndSalesModel::on_timeout, this));
}

DemoTimeAndSalesModel::~DemoTimeAndSalesModel() {
  for(auto& timer : m_query_duration_timers) {
    timer->cancel();
  }
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

QtPromise<void> DemoTimeAndSalesModel::load_older(int max_count) {
  if(m_query_duration != pos_infin) {
    m_resume_update_time = microsec_clock::universal_time() + m_query_duration;
  }
  auto timer = std::make_shared<LiveTimer>(m_query_duration);
  m_query_duration_timers.push_back(timer);
  auto entries = std::vector<Spire::Details::TimeAndSalesEntry>();
  if(m_query_duration != pos_infin) {
    auto timestamp = [&] {
      if(m_entries.get_size() == 0) {
        return microsec_clock::universal_time();
      }
      return m_entries.get(0).m_time_and_sale->m_timestamp - m_period;
    }();
    for(auto i = 0; i < max_count; ++i) {
      entries.insert(entries.begin(), make_entry(timestamp));
      timestamp -= m_period;
    }
  }
  return QtPromise([=, query_duration = m_query_duration] {
    if(query_duration != pos_infin) {
      timer->start();
      timer->wait();
    }
    return entries;
  }, LaunchPolicy::ASYNC).then([=] (auto&& result) {
    auto loaded = std::move(result).get();
    if(m_entries.get_size() != 0) {
      auto front = m_entries.get(0).m_time_and_sale.get_sequence();
      std::erase_if(loaded, [&] (const auto& entry) {
        return entry.m_time_and_sale.get_sequence() >= front;
      });
    }
    m_entries.transact([&] {
      for(auto i = 0; i < std::ssize(loaded); ++i) {
        m_entries.insert(loaded[i], i);
      }
    });
  });
}

int DemoTimeAndSalesModel::get_size() const {
  return m_entries.get_size();
}

const DemoTimeAndSalesModel::Type&
    DemoTimeAndSalesModel::get(int index) const {
  return m_entries.get(index);
}

connection DemoTimeAndSalesModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_entries.connect_operation_signal(slot);
}

void DemoTimeAndSalesModel::transact(
    const std::function<void ()>& transaction) {
  m_entries.transact([&] {
    transaction();
  });
}

Spire::Details::TimeAndSalesEntry DemoTimeAndSalesModel::make_entry(
    ptime timestamp) const {
  if(m_is_data_random) {
    auto random_generator = QRandomGenerator(to_time_t_milliseconds(timestamp));
    return {SequencedValue(
      make_time_and_sale(timestamp,
        truncate_to(Money(random_generator.bounded(2000.0)), Money::CENT),
        random_generator.bounded(1, 10000),
        markets[random_generator.bounded(static_cast<int>(markets.size()))],
        mpids[random_generator.bounded(static_cast<int>(mpids.size()))],
        mpids[random_generator.bounded(static_cast<int>(mpids.size()))]),
      Beam::Sequence(to_time_t_milliseconds(timestamp))),
      static_cast<BboIndicator>(random_generator.bounded(6))};
  }
  return {SequencedValue(make_time_and_sale(
    timestamp, m_price, 100, markets.front(), mpids.front(), mpids.back()),
      Beam::Sequence(to_time_t_milliseconds(timestamp))), m_indicator};
}

void DemoTimeAndSalesModel::on_timeout() {
  if(microsec_clock::universal_time() < m_resume_update_time) {
    return;
  }
  auto entry = make_entry(microsec_clock::universal_time());
  if(m_entries.get_size() == 0 ||
      entry.m_time_and_sale.get_sequence() >
        m_entries.get(get_size() - 1).m_time_and_sale.get_sequence()) {
    m_entries.push(entry);
  }
}
