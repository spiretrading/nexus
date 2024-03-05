#include "Spire/KeyBindings/KeyBindingsModel.hpp"

using namespace Spire;

KeyBindingsModel::KeyBindingsModel()
  : m_order_task_arguments(
      std::make_shared<ArrayListModel<OrderTaskArguments>>()),
    m_cancel_key_bindings(std::make_shared<CancelKeyBindingsModel>()),
    m_interactions_key_bindings(
      std::make_shared<InteractionsKeyBindingsModel>()) {}

const std::shared_ptr<OrderTaskArgumentsListModel>&
    KeyBindingsModel::get_order_task_arguments() const {
  return m_order_task_arguments;
}

const std::shared_ptr<CancelKeyBindingsModel>&
    KeyBindingsModel::get_cancel_key_bindings() const {
  return m_cancel_key_bindings;
}

const std::shared_ptr<InteractionsKeyBindingsModel>&
    KeyBindingsModel::get_interactions_key_bindings() const {
  return m_interactions_key_bindings;
}
