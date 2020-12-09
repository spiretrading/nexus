#include "Spire/KeyBindings/CancelKeySequenceValidationModel.hpp"
#include <QSet>

using namespace Spire;

namespace {
  auto is_modifier_key(Qt::Key key) {
    return key == Qt::Key_Alt || key == Qt::Key_Control ||
      key == Qt::Key_Shift;
  }
}

bool CancelKeySequenceValidationModel::is_valid(
    const QKeySequence& sequence) const {
  switch(sequence.count()) {
    case 1:
      return sequence[0] == Qt::Key_Escape;
    case 2:
      return is_modifier_key(Qt::Key(sequence[0])) &&
        sequence[1] == Qt::Key_Escape;
  }
  return false;
}
