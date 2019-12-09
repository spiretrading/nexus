#include "Spire/SpireTester/TestOrderImbalanceIndicatorModel.hpp"

using namespace Nexus;
using namespace Spire;

TestOrderImbalanceIndicatorModel::LoadEntry::LoadEntry(
  const TimeInterval& interval)
  : m_interval(interval),
    m_is_loaded(false) {}

const TimeInterval&
    TestOrderImbalanceIndicatorModel::LoadEntry::get_interval() const {
  return m_interval;
}

void TestOrderImbalanceIndicatorModel::LoadEntry::set_result(
    std::vector<OrderImbalance> result) {
  auto lock = std::lock_guard(m_mutex);
  m_is_loaded = true;
  m_load_condition.notify_one();
  m_result = std::move(result);
}

std::vector<Nexus::OrderImbalance>&
    TestOrderImbalanceIndicatorModel::LoadEntry::get_result() {
  return m_result;
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    TestOrderImbalanceIndicatorModel::load(
    const TimeInterval& interval) {
  auto load_entry = std::make_shared<LoadEntry>(interval);
  {
    auto lock = std::lock_guard(m_mutex);
    m_load_entries.push_back(load_entry);
    m_load_condition.notify_all();
  }
  return QtPromise([=] {
    auto lock = std::unique_lock(load_entry->m_mutex);
    while(!load_entry->m_is_loaded) {
      load_entry->m_load_condition.wait(lock);
    }
    return std::move(load_entry->get_result());
  }, LaunchPolicy::ASYNC);
}

SubscriptionResult<boost::optional<Nexus::OrderImbalance>>
    TestOrderImbalanceIndicatorModel::subscribe(
    const OrderImbalanceSignal::slot_type& slot) {
  return {boost::signals2::connection(), QtPromise([] {
    return boost::optional<OrderImbalance>();
  })};
}

std::shared_ptr<OrderImbalanceChartModel>
    TestOrderImbalanceIndicatorModel::get_chart_model(
    const Nexus::Security& security) {
  throw std::runtime_error("method not implemented");
}

QtPromise<std::shared_ptr<TestOrderImbalanceIndicatorModel::LoadEntry>>
    TestOrderImbalanceIndicatorModel::pop_load() {
  return QtPromise([=] {
    auto lock = std::unique_lock(m_mutex);
    while(m_load_entries.empty()) {
      m_load_condition.wait(lock);
    }
    auto entry = m_load_entries.front();
    m_load_entries.pop_front();
    return entry;
  }, LaunchPolicy::ASYNC);
}

int TestOrderImbalanceIndicatorModel::get_load_entry_count() const {
  return static_cast<int>(m_load_entries.size());
}
