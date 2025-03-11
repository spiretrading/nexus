#include "Spire/Spire/AnyListValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

AnyListValueModel::AnyListValueModel(
    std::shared_ptr<AnyListModel> source, int index)
    : m_source(std::move(source)),
      m_index(index) {
  if(m_index.get_index() < 0 || m_index.get_index() >= m_source->get_size()) {
    m_source = nullptr;
    m_index.set(-1);
  } else {
    m_current.set(m_source->get(m_index.get_index()));
    m_source_connection = m_source->connect_operation_signal(
      std::bind_front(&AnyListValueModel::on_operation, this));
  }
}

QValidator::State AnyListValueModel::get_state() const {
  if(m_index.get_index() == -1) {
    return QValidator::State::Invalid;
  }
  return m_current.get_state();
}

const AnyListValueModel::Type& AnyListValueModel::get() const {
  return m_current.get();
}

QValidator::State AnyListValueModel::test(const Type& value) const {
  if(m_index.get_index() == -1) {
    return QValidator::State::Invalid;
  }
  return m_current.test(value);
}

QValidator::State AnyListValueModel::set(const Type& value) {
  if(m_index.get_index() == -1) {
    return QValidator::State::Invalid;
  }
  return m_source->set(m_index.get_index(), value);
}

connection AnyListValueModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_current.connect_update_signal(slot);
}

void AnyListValueModel::on_operation(const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::UpdateOperation& operation) {
      if(operation.m_index == m_index.get_index()) {
        m_current.set(operation.m_value);
      }
    },
    [&] (const AnyListModel::PreRemoveOperation& operation) {
      m_index.update(operation);
      if(m_index.get_index() == -1) {
        m_source_connection.disconnect();
        m_source = nullptr;
      }
    },
    [&] (const auto& operation) {
      m_index.update(operation);
    });
}
