#include "Spire/Spire/TaskKeySequenceValidationModel.hpp"

using namespace Spire;

namespace {
  const auto& FUNCTION_KEYS() {
    static auto function_keys = QSet<Qt::Key>({
      Qt::Key_F1,
      Qt::Key_F2,
      Qt::Key_F3,
      Qt::Key_F4,
      Qt::Key_F5,
      Qt::Key_F6,
      Qt::Key_F7,
      Qt::Key_F8,
      Qt::Key_F9,
      Qt::Key_F10,
      Qt::Key_F11,
      Qt::Key_F12,
    });
    return function_keys;
  }
}

TaskKeySequenceValidationModel::TaskKeySequenceValidationModel()
  : m_valid_sequences({ValidKeySequences({FUNCTION_KEYS()}),
      ValidKeySequences({{Qt::Key_Shift, Qt::Key_Alt, Qt::Key_Control},
      FUNCTION_KEYS()})}) {}

bool TaskKeySequenceValidationModel::is_valid(
    const QKeySequence& sequence) const {
  if(sequence.count() == 0) {
    return false;
  }
  for(auto& sequence : m_valid_sequences) {
    for(auto i = std::size_t(0); i < sequence.size(); ++i) {
      if(!sequence[i].contains(sequence[i])) {
        break;
      }
      if(i == sequence.size() - 1) {
        return true;
      }
    }
  }
  return false;
}
