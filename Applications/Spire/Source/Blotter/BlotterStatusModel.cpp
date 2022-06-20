#include "Spire/Blotter/BlotterStatusModel.hpp"

using namespace Nexus;
using namespace Spire;

BlotterStatusModel::BlotterStatusModel(CurrencyId currency,
  ExchangeRateTable exchange_rates, std::shared_ptr<MoneyModel> buying_power,
  std::shared_ptr<MoneyModel> net_loss,
  std::shared_ptr<PortfolioModel> portfolio)
  : m_currency(currency),
    m_exchange_rates(std::move(exchange_rates)),
    m_buying_power(std::move(buying_power)),
    m_net_loss(std::move(net_loss)),
    m_portfolio(std::move(portfolio)),
    m_total_profit_and_loss(std::make_shared<LocalMoneyModel>()),
    m_unrealized_profit_and_loss(std::make_shared<LocalMoneyModel>()),
    m_realized_profit_and_loss(std::make_shared<LocalMoneyModel>()),
    m_fees(std::make_shared<LocalMoneyModel>()),
    m_cost_basis(std::make_shared<LocalMoneyModel>()) {}

const std::shared_ptr<MoneyModel>& BlotterStatusModel::get_buying_power() {
  return m_buying_power;
}

const std::shared_ptr<MoneyModel>& BlotterStatusModel::get_net_loss() {
  return m_net_loss;
}

const std::shared_ptr<MoneyModel>&
    BlotterStatusModel::get_total_profit_and_loss() {
  return m_total_profit_and_loss;
}

const std::shared_ptr<MoneyModel>&
    BlotterStatusModel::get_unrealized_profit_and_loss() {
  return m_unrealized_profit_and_loss;
}

const std::shared_ptr<MoneyModel>&
    BlotterStatusModel::get_realized_profit_and_loss() {
  return m_realized_profit_and_loss;
}

const std::shared_ptr<MoneyModel>& BlotterStatusModel::get_fees() {
  return m_fees;
}

const std::shared_ptr<MoneyModel>& BlotterStatusModel::get_cost_basis() {
  return m_cost_basis;
}
