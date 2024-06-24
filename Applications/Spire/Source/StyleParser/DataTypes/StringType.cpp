#include "Spire/StyleParser/DataTypes/StringType.hpp"

using namespace Spire;

StringType::StringType()
  : DataType("String") {}

bool StringType::is_equal(const DataType& rhs) const {
  return get_name() == rhs.get_name();
}
