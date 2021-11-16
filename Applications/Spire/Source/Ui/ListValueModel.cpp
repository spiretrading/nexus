#include "Spire/Ui/ListValueModel.hpp"

using namespace boost::signals2;
using namespace Spire;

ListValueModel::ListValueModel(std::shared_ptr<ListModel> source, int index)
    : m_source(std::move(source)) {
  if(index < 0 || index >= m_source->get_size()) {
    m_index = -1;
    m_source = nullptr;
  } else {
    m_index = index;
    m_source_connection = m_source->connect_operation_signal(
      std::bind_front(&ListValueModel::on_operation, this));
  }
}

QValidator::State ListValueModel::get_state() const {
  if(m_index == -1) {
    return QValidator::State::Invalid;
  }
  return QValidator::State::Acceptable;
}

const ListValueModel::Type& ListValueModel::get_current() const {
  if(m_index == -1) {
    static auto value = std::any();
    return value;
  }
  return m_source->at(m_index);
}

QValidator::State ListValueModel::set_current(const Type& value) {
  if(m_index == -1) {
    return QValidator::State::Invalid;
  }
  return m_source->set(m_index, value);
}

connection ListValueModel::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_current_signal.connect(slot);
}

void ListValueModel::on_operation(const ListModel::Operation& operation) {
  visit(operation,
    [&] (const ListModel::AddOperation& operation) {
      if(m_index >= operation.m_index) {
        ++m_index;
      }
    },
    [&] (const ListModel::RemoveOperation& operation) {
      if(m_index == operation.m_index) {
        m_source_connection.disconnect();
        m_index = -1;
        m_source = nullptr;
      } else if(m_index > operation.m_index) {
        --m_index;
      }
    },
    [&] (const ListModel::MoveOperation& operation) {
      if(m_index == operation.m_source) {
        m_index = operation.m_destination;
      } else if(operation.m_source < operation.m_destination) {
        if(m_index > operation.m_source && m_index <= operation.m_destination) {
          --m_index;
        }
      } else if(m_index >= operation.m_destination &&
          m_index < operation.m_source) {
        ++m_index;
      }
    },
    [&] (const ListModel::UpdateOperation& operation) {
      if(operation.m_index == m_index) {
        m_current_signal(get_current());
      }
    });
}
