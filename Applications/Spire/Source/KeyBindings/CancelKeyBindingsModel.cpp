#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"

using namespace boost;
using namespace Spire;

CancelKeyBindingsModel::CancelKeyBindingsModel() {
  for(auto i = 0; i < 13; ++i) {
    auto operation = static_cast<Operation>(i);
    m_bindings[operation] = make_validated_value_model<QKeySequence>(
      std::bind_front(&CancelKeyBindingsModel::on_validate, this, operation),
        std::make_shared<LocalKeySequenceValueModel>());
  }
}

std::shared_ptr<KeySequenceValueModel>
    CancelKeyBindingsModel::get_binding(Operation operation) const {
  return m_bindings.at(operation);
}

optional<CancelKeyBindingsModel::Operation>
    CancelKeyBindingsModel::find_operation(const QKeySequence& sequence) const {
  for(auto& binding : m_bindings) {
    if(binding.second->get() == sequence) {
      return binding.first;
    }
  }
  return {};
}

QValidator::State CancelKeyBindingsModel::on_validate(Operation operation,
    const QKeySequence& sequence) {
  if(sequence.count() == 0) {
    return QValidator::Intermediate;
  } else if(sequence.count() > 1) {
    return QValidator::Invalid;
  }
  if(auto search = find_operation(sequence); search && *search != operation) {
    return QValidator::Invalid;
  }
  auto key = sequence[0];
  auto modifier = key & Qt::KeyboardModifierMask;
  key -= modifier;
  if(((modifier == Qt::NoModifier || modifier & Qt::ControlModifier ||
    modifier & Qt::ShiftModifier || modifier & Qt::AltModifier) &&
      ((key >= Qt::Key_F1 && key <= Qt::Key_F12) ||
        (key >= Qt::Key_0 && key <= Qt::Key_9) || key == Qt::Key_Escape))) {
    return QValidator::Acceptable;
  }
  return QValidator::Invalid;
}
