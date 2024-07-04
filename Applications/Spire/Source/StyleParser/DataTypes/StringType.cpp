#include "Spire/StyleParser/DataTypes/StringType.hpp"

using namespace Spire;

StringType::StringType()
  : DataType("String") {}

bool StringType::is_equal(const DataType& rhs) const {
  return dynamic_cast<const StringType*>(&rhs) != nullptr;
}
