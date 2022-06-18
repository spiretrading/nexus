#include "Spire/Blotter/PortfolioModel.hpp"
#include "Spire/Blotter/ValuationModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace Spire;

PortfolioModel::PortfolioModel(MarketDatabase markets,
    const InventorySnapshot& snapshot, std::shared_ptr<OrderListModel> orders,
    std::shared_ptr<ValuationModel> valuation)
    : m_markets(markets),
      m_orders(std::move(orders)),
      m_valuation(std::move(valuation)),
      m_portfolio(std::move(markets)) {
  auto reports = std::vector<std::pair<const Order*, ExecutionReport>>();
  for(auto i = 0; i != m_orders->get_size(); ++i) {
    auto order = m_orders->get(i);
    auto order_reports = optional<std::vector<ExecutionReport>>();
    order->GetPublisher().Monitor(m_tasks.get_slot<ExecutionReport>(
      std::bind_front(&PortfolioModel::on_report, this, std::ref(*order))),
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

const PortfolioModel::Portfolio& PortfolioModel::get_portfolio() const {
  return m_portfolio;
}

connection PortfolioModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

void PortfolioModel::signal_update(const Security& security) {
  auto currency = m_markets.FromCode(security.GetMarket()).m_currency;
  auto update = Portfolio::UpdateEntry();
  update.m_securityInventory =
    m_portfolio.GetBookkeeper().GetInventory(security, currency);
  auto security_entry = m_portfolio.GetSecurityEntries().find(security);
  if(security_entry != m_portfolio.GetSecurityEntries().end()) {
    update.m_unrealizedSecurity = security_entry->second.m_unrealized;
  }
  update.m_currencyInventory = m_portfolio.GetBookkeeper().GetTotal(currency);
  auto currency_entry =
    m_portfolio.GetUnrealizedProfitAndLosses().find(currency);
  if(currency_entry != m_portfolio.GetUnrealizedProfitAndLosses().end()) {
    update.m_unrealizedCurrency = currency_entry->second;
  }
  m_update_signal(update);
}

void PortfolioModel::on_report(
    const Order& order, const ExecutionReport& report) {
  auto& security = order.GetInfo().m_fields.m_security; 
  if(!m_valuation_connections.contains(security)) {
    auto valuation = m_valuation->get_valuation(security);
    auto connection = valuation->connect_update_signal(
      std::bind_front(&PortfolioModel::on_valuation, this, security));
    m_valuation_connections.insert(std::pair(security, std::move(connection)));
  }
  if(m_portfolio.Update(order.GetInfo().m_fields, report)) {
    signal_update(security);
  }
}

void PortfolioModel::on_valuation(
    const Security& security, const SecurityValuation& valuation) {
  auto has_update = [&] {
    if(valuation.m_askValue && valuation.m_bidValue) {
      return m_portfolio.Update(
        security, *valuation.m_askValue, *valuation.m_bidValue);
    } else if(valuation.m_askValue) {
      return m_portfolio.UpdateAsk(security, *valuation.m_askValue);
    } else if(valuation.m_bidValue) {
      return m_portfolio.UpdateBid(security, *valuation.m_bidValue);
    }
  }();
  if(has_update) {
    signal_update(security);
  }
}
