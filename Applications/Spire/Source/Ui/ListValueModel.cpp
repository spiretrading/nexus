#include "Spire/Ui/ListValueModel.hpp"

using namespace boost::signals2;
using namespace Spire;

ListValueModel::ListValueModel(std::shared_ptr<ListModel> source, int index)
  : m_source(std::move(source)),
    m_index(index),
    m_operation_connection(m_source->connect_operation_signal(
      std::bind_front(&ListValueModel::on_operation, this))) {}

QValidator::State ListValueModel::get_state() const {
  if(m_index < 0 || m_index >= m_source->get_size()) {
    return QValidator::State::Invalid;
  }
  return QValidator::State::Acceptable;
}

const ListValueModel::Type& ListValueModel::get_current() const {
  if(get_state() == QValidator::State::Acceptable) {
    return m_source->at(m_index);
  }
  return m_value.get_current();
}

QValidator::State ListValueModel::set_current(const Type& value) {
  return m_source->set(m_index, value);
}

connection ListValueModel::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_current_signal.connect(slot);
}

void ListValueModel::on_operation(const ListModel::Operation& operation) {
  visit(operation,
    [&] (const ListModel::AddOperation& operation) {
      if(m_index >= operation.m_index && m_index < m_source->get_size()) {
        m_current_signal(get_current());
      }
    },
    [&] (const ListModel::RemoveOperation& operation) {
      if(m_index >= operation.m_index && m_index <= m_source->get_size()) {
        m_current_signal(get_current());
      }
    },
    [&] (const ListModel::MoveOperation& operation) {
      auto bounds = std::minmax(operation.m_source, operation.m_destination);
      if(m_index >= bounds.first && m_index <= bounds.second) {
        m_current_signal(get_current());
      }
    },
    [&] (const ListModel::UpdateOperation& operation) {
      if(operation.m_index == m_index) {
        m_current_signal(get_current());
      }
    });
}
