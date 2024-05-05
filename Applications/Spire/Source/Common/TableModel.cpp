#include "Spire/Spire/TableModel.hpp"

using namespace Spire;

QValidator::State TableModel::set(int row, int column, const std::any& value) {
  return QValidator::State::Invalid;
}

QValidator::State TableModel::remove(int row) {
  if(row < 0 || row >= get_row_size()) {
    throw std::out_of_range("The row is out of range.");
  }
  return QValidator::State::Invalid;
}
