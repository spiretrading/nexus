#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/CompositeBlotterStatusModel.hpp"
#include "Spire/Blotter/LocalBlotterPositionsModel.hpp"
#include "Spire/Blotter/LocalBlotterProfitAndLossModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

CompositeBlotterModel::CompositeBlotterModel(std::shared_ptr<TextModel> name,
    std::shared_ptr<BlotterPositionsModel> positions,
    std::shared_ptr<OrderListModel> orders,
    std::shared_ptr<BlotterProfitAndLossModel> profit_and_loss,
    std::shared_ptr<BlotterStatusModel> status)
  : m_name(std::move(name)),
    m_positions(std::move(positions)),
    m_orders(std::move(orders)),
    m_profit_and_loss(std::move(profit_and_loss)),
    m_status(std::move(status)) {}

std::shared_ptr<TextModel> CompositeBlotterModel::get_name() {
  return m_name;
}

std::shared_ptr<BlotterPositionsModel> CompositeBlotterModel::get_positions() {
  return m_positions;
}

std::shared_ptr<OrderListModel> CompositeBlotterModel::get_orders() {
  return m_orders;
}

std::shared_ptr<BlotterProfitAndLossModel>
    CompositeBlotterModel::get_profit_and_loss() {
  return m_profit_and_loss;
}

std::shared_ptr<BlotterStatusModel> CompositeBlotterModel::get_status() {
  return m_status;
}

std::shared_ptr<CompositeBlotterModel> Spire::make_local_blotter_model() {
  return std::make_shared<CompositeBlotterModel>(
    std::make_shared<LocalTextModel>(),
    std::make_shared<LocalBlotterPositionsModel>(),
    std::make_shared<ArrayListModel<Order*>>(),
    std::make_shared<LocalBlotterProfitAndLossModel>(),
    make_local_blotter_status_model());
}
