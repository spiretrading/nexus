#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/CompositeBlotterStatusModel.hpp"
#include "Spire/Blotter/LocalBlotterPositionsModel.hpp"
#include "Spire/Blotter/LocalBlotterProfitAndLossModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/MultiSelectionModel.hpp"

using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

CompositeBlotterModel::CompositeBlotterModel(std::shared_ptr<TextModel> name,
    std::shared_ptr<BooleanModel> is_active,
    std::shared_ptr<BooleanModel> is_pinned,
    std::shared_ptr<BlotterTaskListModel> tasks,
    std::shared_ptr<ListModel<int>> task_selection,
    std::shared_ptr<BlotterPositionsModel> positions,
    std::shared_ptr<BlotterProfitAndLossModel> profit_and_loss,
    std::shared_ptr<BlotterStatusModel> status)
  : m_name(std::move(name)),
    m_is_active(std::move(is_active)),
    m_is_pinned(std::move(is_pinned)),
    m_tasks(std::move(tasks)),
    m_task_selection(std::move(task_selection)),
    m_positions(std::move(positions)),
    m_profit_and_loss(std::move(profit_and_loss)),
    m_status(std::move(status)) {}

std::shared_ptr<TextModel> CompositeBlotterModel::get_name() {
  return m_name;
}

std::shared_ptr<BooleanModel> CompositeBlotterModel::is_active() {
  return m_is_active;
}

std::shared_ptr<BooleanModel> CompositeBlotterModel::is_pinned() {
  return m_is_pinned;
}

std::shared_ptr<BlotterTaskListModel> CompositeBlotterModel::get_tasks() {
  return m_tasks;
}

std::shared_ptr<ListModel<int>>
    CompositeBlotterModel::get_task_selection() const {
  return m_task_selection;
}

std::shared_ptr<BlotterPositionsModel> CompositeBlotterModel::get_positions() {
  return m_positions;
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
    std::make_shared<LocalBooleanModel>(),
    std::make_shared<LocalBooleanModel>(),
    std::make_shared<ArrayListModel<std::shared_ptr<BlotterTaskEntry>>>(),
    std::make_shared<ListMultiSelectionModel>(),
    std::make_shared<LocalBlotterPositionsModel>(),
    std::make_shared<LocalBlotterProfitAndLossModel>(),
    make_local_blotter_status_model());
}

std::shared_ptr<CompositeBlotterModel> Spire::make_derived_blotter_model(
    std::shared_ptr<TextModel> name, std::shared_ptr<BooleanModel> is_active,
    std::shared_ptr<BooleanModel> is_pinned,
    std::shared_ptr<BlotterTaskListModel> tasks) {
  return std::make_shared<CompositeBlotterModel>(std::move(name),
    std::move(is_active), std::move(is_pinned), tasks,
    std::make_shared<ListMultiSelectionModel>(),
    std::make_shared<LocalBlotterPositionsModel>(),
    std::make_shared<LocalBlotterProfitAndLossModel>(),
    make_local_blotter_status_model());
}