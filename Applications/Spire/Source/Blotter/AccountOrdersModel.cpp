#include "Spire/Blotter/AccountOrdersModel.hpp"
#include <functional>
#include "Nexus/OrderExecutionService/StandardQueries.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

AccountOrdersModel::AccountOrdersModel(
    const DirectoryEntry& account, Clients& clients) {
  auto now = clients.get_time_client().get_time();
  auto query = AccountQuery();
  query.set_index(account);
  query.set_range(Range::REAL_TIME);
  query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
  query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
  clients.get_order_execution_client().query(
    query, m_events.get_slot<std::shared_ptr<Order>>(
      std::bind_front(&AccountOrdersModel::on_live_order, this)));
  m_history = query_daily_order_submissions(
    account, now, now, clients.get_order_execution_client(),
    m_events.get_slot<std::shared_ptr<Order>>(
      std::bind_front(&AccountOrdersModel::on_order, this)));
}

connection AccountOrdersModel::connect_live_signal(
    const LiveSignal::slot_type& slot) const {
  return m_live_signal.connect(slot);
}

void AccountOrdersModel::on_order(const std::shared_ptr<Order>& order) {
  if(m_ids.insert(order->get_info().m_id).second) {
    push(order);
  }
}

void AccountOrdersModel::on_live_order(const std::shared_ptr<Order>& order) {
  on_order(order);
  m_live_signal(order);
}
