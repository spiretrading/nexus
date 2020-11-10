#include "Spire/Spire/TaskKeySequenceValidationModel.hpp"
#include <QSet>

using namespace Spire;

namespace {
  auto MODIFIER_KEYS() {
    static auto keys = QSet<Qt::Key>({Qt::Key_Alt, Qt::Key_Control,
      Qt::Key_Shift});
    return keys;
  }

  auto FUNCTION_KEYS() {
    static auto keys = QSet<Qt::Key>({Qt::Key_F1, Qt::Key_F2, Qt::Key_F3,
      Qt::Key_F4, Qt::Key_F5, Qt::Key_F6, Qt::Key_F7, Qt::Key_F8, Qt::Key_F9,
      Qt::Key_F10, Qt::Key_F11, Qt::Key_F12});
    return keys;
  }
}

bool TaskKeySequenceValidationModel::is_valid(
    const QKeySequence& sequence) const {
  switch(sequence.count()) {
    case 1:
      return FUNCTION_KEYS().contains(Qt::Key(sequence[0]));
    case 2:
      return MODIFIER_KEYS().contains(Qt::Key(sequence[0])) &&
        FUNCTION_KEYS().contains(Qt::Key(sequence[1]));
  }
  return false;
}
