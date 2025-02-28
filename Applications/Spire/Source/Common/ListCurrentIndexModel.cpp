#include "Spire/Spire/ListCurrentIndexModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

ListCurrentIndexModel::ListCurrentIndexModel(std::shared_ptr<AnyListModel> list)
  : ListCurrentIndexModel(std::move(list), none) {}

ListCurrentIndexModel::ListCurrentIndexModel(
    std::shared_ptr<AnyListModel> list, optional<int> index)
    : m_list(std::move(list)),
      m_index(index),
      m_tracker(index.value_or(-1)) {
  if(m_index.get() &&
      (*m_index.get() < 0 || *m_index.get() > m_list->get_size())) {
    m_index.set(none);
    m_tracker.set(-1);
  }
  m_connection = m_list->connect_operation_signal(
    std::bind_front(&ListCurrentIndexModel::on_operation, this));
}

const ListCurrentIndexModel::Type& ListCurrentIndexModel::get() const {
  return m_index.get();
}

QValidator::State ListCurrentIndexModel::test(const Type& value) const {
  if(value == none || value >= 0 && value < m_list->get_size()) {
    return QValidator::Acceptable;
  }
  return QValidator::Invalid;
}

QValidator::State ListCurrentIndexModel::set(const Type& value) {
  if(test(value) == QValidator::Invalid) {
    return QValidator::Invalid;
  }
  m_tracker.set(value.value_or(-1));
  return m_index.set(value);
}

connection ListCurrentIndexModel::connect_update_signal(
    const typename UpdateSignal::slot_type& slot) const {
  return m_index.connect_update_signal(slot);
}

void ListCurrentIndexModel::on_operation(
    const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::PreRemoveOperation& operation) {
      if(auto index = get()) {
        m_tracker.update(operation);
        if(m_list->get_size() == 0) {
          m_tracker.set(-1);
          m_index.set(none);
        } else if(index >= m_list->get_size()) {
          m_tracker.set(m_list->get_size() - 1);
          m_index.set(m_list->get_size() - 1);
        } else if(operation.m_index == *index) {
          m_tracker.set(*index);
          m_index.set(*index);
        } else if(m_tracker.get_index() == -1) {
          m_index.set(none);
        } else {
          m_index.set(m_tracker.get_index());
        }
      }
    },
    [&] (const auto& operation) {
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
