#include "Spire/Blotter/CompositeBlotterModel.hpp"

using namespace Spire;

CompositeBlotterModel::CompositeBlotterModel(
  std::shared_ptr<TextModel> name, std::shared_ptr<BooleanModel> is_active,
  std::shared_ptr<BooleanModel> is_pinned,
  std::shared_ptr<TasksModel> tasks) noexcept
  : m_name(std::move(name)),
    m_is_active(std::move(is_active)),
    m_is_pinned(std::move(is_pinned)),
    m_tasks(std::move(tasks)) {}

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
