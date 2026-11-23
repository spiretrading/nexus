#include "Spire/Blotter/CompositeBlotterModel.hpp"

using namespace Beam;
using namespace Spire;

CompositeBlotterModel::CompositeBlotterModel(
  DirectoryEntry account, bool is_consolidated,
  std::shared_ptr<TextModel> name, std::shared_ptr<BooleanModel> is_active,
  std::shared_ptr<BooleanModel> is_pinned,
  std::shared_ptr<TasksModel> tasks,
  std::shared_ptr<TaskMonitorsModel> monitors,
  std::shared_ptr<OrdersModel> orders,
  std::shared_ptr<PortfolioModel> portfolio) noexcept
  : m_account(std::move(account)),
    m_is_consolidated(is_consolidated),
    m_name(std::move(name)),
    m_is_active(std::move(is_active)),
    m_is_pinned(std::move(is_pinned)),
    m_tasks(std::move(tasks)),
    m_monitors(std::move(monitors)),
    m_orders(std::move(orders)),
    m_portfolio(std::move(portfolio)) {}

const DirectoryEntry& CompositeBlotterModel::get_account() const {
  return m_account;
}

bool CompositeBlotterModel::is_consolidated() const {
  return m_is_consolidated;
}

const std::shared_ptr<TextModel>& CompositeBlotterModel::get_name() const {
  return m_name;
}

const std::shared_ptr<BooleanModel>& CompositeBlotterModel::is_active() const {
  return m_is_active;
}

const std::shared_ptr<BooleanModel>& CompositeBlotterModel::is_pinned() const {
  return m_is_pinned;
}

const std::shared_ptr<TasksModel>& CompositeBlotterModel::get_tasks() const {
  return m_tasks;
}

const std::shared_ptr<TaskMonitorsModel>&
    CompositeBlotterModel::get_task_monitors() const {
  return m_monitors;
}

const std::shared_ptr<OrdersModel>& CompositeBlotterModel::get_orders() const {
  return m_orders;
}

const std::shared_ptr<PortfolioModel>&
    CompositeBlotterModel::get_portfolio() const {
  return m_portfolio;
}
