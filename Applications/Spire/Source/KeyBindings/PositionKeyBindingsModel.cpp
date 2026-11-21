#include "Spire/KeyBindings/PositionKeyBindingsModel.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"

using namespace boost;
using namespace Spire;

PositionKeyBindingsModel::PositionKeyBindingsModel() {
  for(auto i = 0; i != OPERATION_COUNT; ++i) {
    m_bindings[i] = make_validated_value_model(
      [] (const QKeySequence& sequence) {
        if(sequence.isEmpty()) {
          return QValidator::Intermediate;
        } else if(sequence.count() > 1) {
          return QValidator::Invalid;
        }
        return QValidator::Acceptable;
      }, std::make_shared<LocalKeySequenceValueModel>());
    m_connections[i] = m_bindings[i]->connect_update_signal(std::bind_front(
      &PositionKeyBindingsModel::on_update, this, static_cast<Operation>(i)));
  }
  get_binding(Operation::FLATTEN)->set(Qt::CTRL | Qt::Key_F);
  get_binding(Operation::FLATTEN_ALL)->set(Qt::CTRL | Qt::SHIFT | Qt::Key_F);
}

std::shared_ptr<KeySequenceValueModel>
    PositionKeyBindingsModel::get_binding(Operation operation) const {
  return m_bindings[static_cast<int>(operation)];
}

optional<PositionKeyBindingsModel::Operation>
    PositionKeyBindingsModel::find_operation(const QKeySequence& sequence)
      const {
  if(sequence.isEmpty()) {
    return none;
  }
  for(auto i = 0; i != OPERATION_COUNT; ++i) {
    if(m_bindings[i]->get() == sequence) {
      return static_cast<Operation>(i);
    }
  }
  return none;
}

void PositionKeyBindingsModel::on_update(
    Operation operation, const QKeySequence& sequence) {
  if(sequence.isEmpty()) {
    return;
  }
  for(auto i = 0; i != OPERATION_COUNT; ++i) {
    if(i != static_cast<int>(operation) && m_bindings[i]->get() == sequence) {
      m_bindings[i]->set(QKeySequence());
    }
  }
}
