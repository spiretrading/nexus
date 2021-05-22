#include "Spire/Ui/ListModel.hpp"

using namespace Spire;

QValidator::State ListModel::set(int index, const std::any& value) {
  return QValidator::State::Invalid;
}
