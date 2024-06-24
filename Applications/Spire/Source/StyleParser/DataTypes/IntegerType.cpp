#include "Spire/StyleParser/DataTypes/IntegerType.hpp"

using namespace Spire;

IntegerType::IntegerType()
  : DataType("Integer") {}

bool IntegerType::is_equal(const DataType& rhs) const {
  return get_name() == rhs.get_name();
}
