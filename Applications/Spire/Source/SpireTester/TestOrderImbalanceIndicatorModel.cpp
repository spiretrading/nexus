#include "Spire/SpireTester/TestOrderImbalanceIndicatorModel.hpp"
#include <mutex>
#include "Spire/Spire/QtPromise.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

TestOrderImbalanceIndicatorModel::SubscribeEntry::SubscribeEntry(
  const ptime& start,
  const ptime& end)
    : m_start(start),
      m_end(end),
      m_is_loaded(false) {}

const ptime&
    TestOrderImbalanceIndicatorModel::SubscribeEntry::get_start() const {
  return m_start;
}

const ptime&
    TestOrderImbalanceIndicatorModel::SubscribeEntry::get_end() const {
  return m_end;
}

void TestOrderImbalanceIndicatorModel::SubscribeEntry::set_result(
    std::vector<OrderImbalance> result) {
  auto lock = std::lock_guard(m_mutex);
  m_is_loaded = true;
  m_subscribe_condition.notify_one();
  m_result = std::move(result);
}

std::vector<Nexus::OrderImbalance>&
    TestOrderImbalanceIndicatorModel::SubscribeEntry::get_result() {
  return m_result;
}

std::tuple<boost::signals2::connection,
    QtPromise<std::vector<Nexus::OrderImbalance>>>
    TestOrderImbalanceIndicatorModel::subscribe(const ptime& start,
      const ptime& end, const OrderImbalanceSignal::slot_type& slot) {
  auto subscribe_entry = std::make_shared<SubscribeEntry>(start, end);
  {
    auto lock = std::lock_guard(m_mutex);
    m_subscribe_entries.push_back(subscribe_entry);
    m_subscribe_condition.notify_all();
  }
  return {boost::signals2::connection(), QtPromise([=] {
    auto lock = std::unique_lock(subscribe_entry->m_mutex);
    while(!subscribe_entry->m_is_loaded) {
      subscribe_entry->m_subscribe_condition.wait(lock);
    }
    return std::move(subscribe_entry->get_result());
  }, LaunchPolicy::ASYNC)};
}

QtPromise<std::shared_ptr<TestOrderImbalanceIndicatorModel::SubscribeEntry>>
    TestOrderImbalanceIndicatorModel::pop_subscribe() {
  return QtPromise([=] {
    auto lock = std::unique_lock(m_mutex);
    while(m_subscribe_entries.empty()) {
      m_subscribe_condition.wait(lock);
    }
    auto entry = m_subscribe_entries.front();
    m_subscribe_entries.pop_front();
    return entry;
  }, LaunchPolicy::ASYNC);
}

int TestOrderImbalanceIndicatorModel::get_subscribe_entry_count() const {
  return static_cast<int>(m_subscribe_entries.size());
}
