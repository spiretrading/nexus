#include "Spire/SpireTester/TestOrderImbalanceIndicatorModel.hpp"
#include <mutex>
#include "Spire/Spire/QtPromise.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

TestOrderImbalanceIndicatorModel::SubscriptionEntry::SubscriptionEntry(
    ptime start, ptime end)
  : m_start(start),
    m_end(end),
    m_is_loaded(false) {}

ptime TestOrderImbalanceIndicatorModel::SubscriptionEntry::get_start() const {
  return m_start;
}

ptime TestOrderImbalanceIndicatorModel::SubscriptionEntry::get_end() const {
  return m_end;
}

void TestOrderImbalanceIndicatorModel::SubscriptionEntry::set_result(
    std::vector<OrderImbalance> result) {
  auto lock = std::lock_guard(m_mutex);
  m_is_loaded = true;
  m_subscription_condition.notify_one();
  m_result = std::move(result);
}

std::vector<Nexus::OrderImbalance>&
    TestOrderImbalanceIndicatorModel::SubscriptionEntry::get_result() {
  return m_result;
}

OrderImbalanceIndicatorModel::SubscriptionResult
    TestOrderImbalanceIndicatorModel::subscribe(ptime start, ptime end,
    const OrderImbalanceSignal::slot_type& slot) {
  auto subscription_entry = std::make_shared<SubscriptionEntry>(start, end);
  {
    auto lock = std::lock_guard(m_mutex);
    m_subscription_entries.push_back(subscription_entry);
    m_subscription_condition.notify_all();
  }
  return {boost::signals2::connection(), QtPromise([=] {
    auto lock = std::unique_lock(subscription_entry->m_mutex);
    while(!subscription_entry->m_is_loaded) {
      subscription_entry->m_subscription_condition.wait(lock);
    }
    return std::move(subscription_entry->get_result());
  }, LaunchPolicy::ASYNC)};
}

QtPromise<std::shared_ptr<TestOrderImbalanceIndicatorModel::SubscriptionEntry>>
    TestOrderImbalanceIndicatorModel::pop_subscription() {
  return QtPromise([=] {
    auto lock = std::unique_lock(m_mutex);
    while(m_subscription_entries.empty()) {
      m_subscription_condition.wait(lock);
    }
    auto entry = m_subscription_entries.front();
    m_subscription_entries.pop_front();
    return entry;
  }, LaunchPolicy::ASYNC);
}

int TestOrderImbalanceIndicatorModel::get_subscription_entry_count() const {
  return static_cast<int>(m_subscription_entries.size());
}
