#include "Spire/ReplayUiTester/GeneratedTimeAndSalesModel.hpp"
#include <QRandomGenerator>
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto EPOCH = time_from_string("1970-01-01 00:00:00");
  const auto MARKETS = std::vector<std::string>{"XNYS", "TSE", "CHD", "CHI"};
  const auto MPIDS = std::vector<std::string>{"NBFI", "CIBC", "PFSS", "RBCD"};

  auto to_milliseconds(ptime timestamp) {
    return (timestamp - EPOCH).total_milliseconds();
  }

  auto to_timestamp(std::int64_t milliseconds) {
    return EPOCH + posix_time::milliseconds(milliseconds);
  }
}

GeneratedTimeAndSalesModel::GeneratedTimeAndSalesModel(
    Ticker ticker, ptime start, time_duration period)
    : m_ticker(std::move(ticker)),
      m_start(start),
      m_period(period),
      m_seed(static_cast<int>(qHash(to_text(m_ticker)))),
      m_price(10 + std::abs(m_seed) % 90),
      m_index(get_index(microsec_clock::universal_time())) {
  QObject::connect(&m_timer, &QTimer::timeout,
    std::bind_front(&GeneratedTimeAndSalesModel::on_timeout, this));
  m_timer.start(static_cast<int>(m_period.total_milliseconds()));
}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    GeneratedTimeAndSalesModel::query_until(
      Beam::Sequence sequence, int max_count) {
  auto now = microsec_clock::universal_time();
  auto last = [&] {
    if(sequence >= Beam::Sequence(to_milliseconds(now))) {
      return get_index(now);
    }
    return get_index(to_timestamp(sequence.get_ordinal()));
  }();
  auto result = std::vector<Entry>();
  auto first = std::max(0, last - max_count + 1);
  for(auto i = first; i <= last; ++i) {
    result.push_back(make_entry(i));
  }
  return result;
}

connection GeneratedTimeAndSalesModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

int GeneratedTimeAndSalesModel::get_index(ptime timestamp) const {
  if(timestamp <= m_start) {
    return -1;
  }
  return static_cast<int>((timestamp - m_start).total_milliseconds() /
    m_period.total_milliseconds());
}

TimeAndSalesModel::Entry GeneratedTimeAndSalesModel::make_entry(
    int index) const {
  auto generator = QRandomGenerator(m_seed + index);
  auto timestamp = m_start + m_period * index;
  auto price = truncate_to(
    Money(m_price + 0.01 * generator.bounded(-50, 51)), Money::CENT);
  auto time_and_sale = TimeAndSale(timestamp, price,
    100 * generator.bounded(1, 20),
    TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
    MARKETS[generator.bounded(static_cast<int>(MARKETS.size()))],
    MPIDS[generator.bounded(static_cast<int>(MPIDS.size()))],
    MPIDS[generator.bounded(static_cast<int>(MPIDS.size()))]);
  return Entry(SequencedValue(std::move(time_and_sale),
    Beam::Sequence(to_milliseconds(timestamp))),
    static_cast<BboIndicator>(generator.bounded(1, 6)));
}

void GeneratedTimeAndSalesModel::on_timeout() {
  auto index = get_index(microsec_clock::universal_time());
  while(m_index < index) {
    ++m_index;
    m_update_signal(make_entry(m_index));
  }
}
