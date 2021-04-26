#include "Spire/Ui/TableModel.hpp"

using namespace Spire;

QValidator::State TableModel::set(int row, int column, const std::any& value) {
  return QValidator::State::Invalid;
}
