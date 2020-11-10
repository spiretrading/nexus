#include "Spire/Spire/CancelKeySequenceValidationModel.hpp"
#include <QSet>

using namespace Spire;

namespace {
  auto MODIFIER_KEYS() {
    static auto keys = QSet<Qt::Key>({Qt::Key_Alt, Qt::Key_Control,
      Qt::Key_Shift});
    return keys;
  }
}

bool CancelKeySequenceValidationModel::is_valid(
    const QKeySequence& sequence) const {
  switch(sequence.count()) {
    case 1:
      return sequence[0] == Qt::Key_Escape;
    case 2:
      return MODIFIER_KEYS().contains(Qt::Key(sequence[0])) &&
        sequence[1] == Qt::Key_Escape;
  }
  return false;
}
