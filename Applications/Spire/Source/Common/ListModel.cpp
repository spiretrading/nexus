#include "Spire/Spire/ListModel.hpp"

using namespace Spire;

std::any AnyListModel::get(int index) const {
  return at(index);
}
