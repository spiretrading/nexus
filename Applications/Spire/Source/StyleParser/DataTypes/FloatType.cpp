#include "Spire/StyleParser/DataTypes/FloatType.hpp"

using namespace Spire;

FloatType::FloatType()
  : DataType("Float") {}

bool FloatType::is_equal(const DataType& rhs) const {
  return get_name() == rhs.get_name();
}
