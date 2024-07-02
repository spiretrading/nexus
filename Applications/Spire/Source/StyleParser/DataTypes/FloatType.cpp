#include "Spire/StyleParser/DataTypes/FloatType.hpp"

using namespace Spire;

FloatType::FloatType()
  : DataType("Float") {}

bool FloatType::is_equal(const DataType& rhs) const {
  return dynamic_cast<const FloatType*>(&rhs) != nullptr;
}
