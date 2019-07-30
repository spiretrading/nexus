#include "Spire/SpireTester/TestChartModel.hpp"
#include <mutex>

using namespace Beam::Queries;
using namespace boost::signals2;
using namespace Spire;

TestChartModel::LoadEntry::LoadEntry(ChartValue first, ChartValue last,
    const SnapshotLimit& limit)
    : m_first(first),
      m_last(last),
      m_limit(limit),
      m_is_loaded(false) {}

ChartValue TestChartModel::LoadEntry::get_first() const {
  return m_first;
}

ChartValue TestChartModel::LoadEntry::get_last() const {
  return m_last;
}

const SnapshotLimit& TestChartModel::LoadEntry::get_limit() const {
  return m_limit;
}

void TestChartModel::LoadEntry::set_result(std::vector<Candlestick> result) {
  auto lock = std::lock_guard(m_mutex);
  m_is_loaded = true;
  m_load_condition.notify_one();
  m_result = std::move(result);
}

std::vector<Candlestick>& TestChartModel::LoadEntry::get_result() {
  return m_result;
}

TestChartModel::TestChartModel(ChartValue::Type x_axis_type,
    ChartValue::Type y_axis_type)
    : m_x_axis_type(x_axis_type),
      m_y_axis_type(y_axis_type) {}

std::shared_ptr<TestChartModel::LoadEntry> TestChartModel::pop_load() {
  auto entry = m_load_entries.front();
  m_load_entries.pop_front();
  return entry;
}

ChartValue::Type TestChartModel::get_x_axis_type() const {
  return m_x_axis_type;
}

ChartValue::Type TestChartModel::get_y_axis_type() const {
  return m_y_axis_type;
}

QtPromise<std::vector<Candlestick>> TestChartModel::load(ChartValue first,
    ChartValue last, const SnapshotLimit& limit) {
  auto load_entry = std::make_shared<LoadEntry>(first, last, limit);
  m_load_entries.push_back(load_entry);
  return QtPromise([=] {
    auto lock = std::unique_lock(load_entry->m_mutex);
    while(!load_entry->m_is_loaded) {
      load_entry->m_load_condition.wait(lock);
    }
    return std::move(load_entry->get_result());
  }, LaunchPolicy::ASYNC);
}

connection TestChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_candlestick_signal.connect(slot);
}
