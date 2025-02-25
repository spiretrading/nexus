#include "Spire/Spire/ListIndexModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

ListIndexModel::ListIndexModel(std::shared_ptr<AnyListModel> list)
  : ListIndexModel(std::move(list), none) {}

ListIndexModel::ListIndexModel(
    std::shared_ptr<AnyListModel> list, optional<int> index)
    : m_list(std::move(list)),
      m_index(index),
      m_tracker(index.value_or(-1)) {
  m_list_connection = m_list->connect_operation_signal(
    std::bind_front(&ListIndexModel::on_operation, this));
}

const ListIndexModel::Type& ListIndexModel::get() const {
  return m_index.get();
}

QValidator::State ListIndexModel::test(const Type& value) const {
  if(value == none || value >= 0 && value < m_list->get_size()) {
    return QValidator::Acceptable;
  }
  return QValidator::Invalid;
}

QValidator::State ListIndexModel::set(const Type& value) {
  if(test(value) == QValidator::Invalid) {
    return QValidator::Invalid;
  }
  m_tracker.set(value.value_or(-1));
  return m_index.set(value);
}

connection ListIndexModel::connect_update_signal(
    const typename UpdateSignal::slot_type& slot) const {
  return m_index.connect_update_signal(slot);
}

void ListIndexModel::on_operation(const AnyListModel::Operation& operation) {
  // TODO: If operation is a remove, always signal an update.
  visit(operation, [&] (const auto& operation) {
    auto index = get();
    m_tracker.update(operation);
    if(m_tracker.get_index() != index.value_or(-1)) {
      if(m_tracker.get_index() == -1) {
        m_index.set(none);
      } else {
        m_index.set(m_tracker.get_index());
      }
    }
  });
}
