#include "Spire/Spire/TaskKeySequenceValidationModel.hpp"
#include <QSet>

using namespace Spire;

namespace {
  auto is_modifier_key(Qt::Key key) {
    return key == Qt::Key_Alt || key == Qt::Key_Control ||
      key == Qt::Key_Shift;
  }

  auto is_function_key(Qt::Key key) {
    return key >= Qt::Key_F1 && key <= Qt::Key_F12;
  }
}

bool TaskKeySequenceValidationModel::is_valid(
    const QKeySequence& sequence) const {
  switch(sequence.count()) {
    case 1:
      return is_function_key(Qt::Key(sequence[0]));
    case 2:
      return is_modifier_key(Qt::Key(sequence[0])) &&
        is_function_key(Qt::Key(sequence[1]));
  }
  return false;
}
