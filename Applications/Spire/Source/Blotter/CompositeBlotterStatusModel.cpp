#include "Spire/Blotter/CompositeBlotterStatusModel.hpp"

using namespace Spire;

CompositeBlotterStatusModel::CompositeBlotterStatusModel(
  std::shared_ptr<MoneyModel> buying_power,
  std::shared_ptr<MoneyModel> net_loss,
  std::shared_ptr<MoneyModel> total_profit_and_loss,
  std::shared_ptr<MoneyModel> unrealized_profit_and_loss,
  std::shared_ptr<MoneyModel> realized_profit_and_loss,
  std::shared_ptr<MoneyModel> fees, std::shared_ptr<MoneyModel> cost_basis)
  : m_buying_power(std::move(buying_power)),
    m_net_loss(std::move(net_loss)),
    m_total_profit_and_loss(std::move(total_profit_and_loss)),
    m_unrealized_profit_and_loss(std::move(unrealized_profit_and_loss)),
    m_realized_profit_and_loss(std::move(realized_profit_and_loss)),
    m_fees(std::move(fees)),
    m_cost_basis(std::move(cost_basis)) {}

std::shared_ptr<MoneyModel> CompositeBlotterStatusModel::get_buying_power() {
  return m_buying_power;
}

std::shared_ptr<MoneyModel> CompositeBlotterStatusModel::get_net_loss() {
  return m_net_loss;
}

std::shared_ptr<MoneyModel>
    CompositeBlotterStatusModel::get_total_profit_and_loss() {
  return m_total_profit_and_loss;
}

std::shared_ptr<MoneyModel>
    CompositeBlotterStatusModel::get_unrealized_profit_and_loss() {
  return m_unrealized_profit_and_loss;
}

std::shared_ptr<MoneyModel>
    CompositeBlotterStatusModel::get_realized_profit_and_loss() {
  return m_realized_profit_and_loss;
}

std::shared_ptr<MoneyModel> CompositeBlotterStatusModel::get_fees() {
  return m_fees;
}

std::shared_ptr<MoneyModel> CompositeBlotterStatusModel::get_cost_basis() {
  return m_cost_basis;
}

std::shared_ptr<CompositeBlotterStatusModel>
    Spire::make_local_blotter_status_model() {
  return std::make_shared<CompositeBlotterStatusModel>(
    std::make_shared<LocalMoneyModel>(), std::make_shared<LocalMoneyModel>(),
    std::make_shared<LocalMoneyModel>(), std::make_shared<LocalMoneyModel>(),
    std::make_shared<LocalMoneyModel>(), std::make_shared<LocalMoneyModel>(),
    std::make_shared<LocalMoneyModel>());
}
