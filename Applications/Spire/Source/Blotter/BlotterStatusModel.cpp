#include "Spire/Blotter/BlotterStatusModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  void update_model(MoneyModel& model, CurrencyId currency,
      CurrencyId base_currency, const ExchangeRateTable& exchange_rates,
      Money value, Money& total) {
    auto base_value = exchange_rates.Convert(value, currency, base_currency);
    auto delta = base_value - total;
    total = base_value;
    model.set(model.get() + delta);
  }
}

BlotterStatusModel::BlotterStatusModel(CurrencyId currency,
  ExchangeRateTable exchange_rates, std::shared_ptr<MoneyModel> buying_power,
  std::shared_ptr<MoneyModel> net_loss,
  std::shared_ptr<PortfolioModel> portfolio)
  : m_currency(currency),
    m_exchange_rates(std::move(exchange_rates)),
    m_buying_power(std::move(buying_power)),
    m_net_loss(std::move(net_loss)),
    m_portfolio(std::move(portfolio)),
    m_portfolio_connection(m_portfolio->connect_update_signal(
      std::bind_front(&BlotterStatusModel::on_portfolio, this))),
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

const std::shared_ptr<MoneyModel>& BlotterStatusModel::get_cost_basis() {
  return m_cost_basis;
}

const std::shared_ptr<MoneyModel>&
    BlotterStatusModel::get_unrealized_profit_and_loss() {
  return m_unrealized_profit_and_loss;
}

const std::shared_ptr<MoneyModel>& BlotterStatusModel::get_fees() {
  return m_fees;
}

const std::shared_ptr<MoneyModel>&
    BlotterStatusModel::get_realized_profit_and_loss() {
  return m_realized_profit_and_loss;
}

const std::shared_ptr<MoneyModel>&
    BlotterStatusModel::get_total_profit_and_loss() {
  return m_total_profit_and_loss;
}

void BlotterStatusModel::on_portfolio(
    const PortfolioModel::Portfolio::UpdateEntry& update) {
  auto currency = update.m_currencyInventory.m_position.m_key.m_currency;
  update_model(*m_cost_basis, currency, m_currency, m_exchange_rates,
    update.m_currencyInventory.m_position.m_costBasis,
    m_cost_basis_totals[currency]);
  update_model(*m_unrealized_profit_and_loss, currency, m_currency,
    m_exchange_rates, update.m_unrealizedCurrency,
    m_unrealized_totals[currency]);
  update_model(*m_fees, currency, m_currency, m_exchange_rates,
    update.m_currencyInventory.m_fees, m_fee_totals[currency]);
  update_model(*m_realized_profit_and_loss, currency, m_currency,
    m_exchange_rates, update.m_currencyInventory.m_grossProfitAndLoss -
    update.m_currencyInventory.m_fees, m_realized_totals[currency]);
  m_total_profit_and_loss->set(m_unrealized_profit_and_loss->get() +
    m_realized_profit_and_loss->get() - m_fees->get());
}
