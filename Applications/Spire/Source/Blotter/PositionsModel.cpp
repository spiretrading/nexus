#include "Spire/Blotter/PositionsModel.hpp"
#include "Spire/Blotter/ValuationModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace Spire;

PositionsModel::PositionsModel(MarketDatabase markets,
    const InventorySnapshot& snapshot, std::shared_ptr<OrderListModel> orders,
    std::shared_ptr<ValuationModel> valuation)
    : m_orders(std::move(orders)),
      m_valuation(std::move(valuation)),
      m_portfolio(std::move(markets)) {
  auto reports = std::vector<std::pair<const Order*, ExecutionReport>>();
  for(auto i = 0; i != m_orders->get_size(); ++i) {
    auto order = m_orders->get(i);
    auto order_reports = optional<std::vector<ExecutionReport>>();
    order->GetPublisher().Monitor(m_tasks.get_slot<ExecutionReport>(
      std::bind_front(&PositionsModel::on_report, this, std::ref(*order))),
      Store(order_reports));
    if(order_reports) {
      for(auto& report : *order_reports) {
        reports.push_back(std::pair(order, report));
      }
    }
  }
  std::sort(reports.begin(), reports.end(), [] (auto& left, auto& right) {
    return std::tie(
      left.second.m_timestamp, left.first->GetInfo().m_timestamp) <
        std::tie(right.second.m_timestamp, right.first->GetInfo().m_timestamp);
  });
  for(auto& report : reports) {
    on_report(*report.first, report.second);
  }
}

std::vector<PositionsModel::Entry> PositionsModel::get_positions() const {
  auto positions = std::vector<Entry>();
  for(auto& position : m_positions) {
    positions.push_back(position.second);
  }
  return positions;
}

connection PositionsModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

void PositionsModel::on_report(
    const Order& order, const ExecutionReport& report) {
  auto& security = order.GetInfo().m_fields.m_security; 
  if(!m_valuation_connections.contains(security)) {
    auto valuation = m_valuation->get_valuation(security);
    auto connection = valuation->connect_update_signal(
      std::bind_front(&PositionsModel::on_valuation, this, security));
    m_valuation_connections.insert(std::pair(security, std::move(connection)));
  }
}

void PositionsModel::on_valuation(
    const Security& security, const SecurityValuation& valuation) {
}
