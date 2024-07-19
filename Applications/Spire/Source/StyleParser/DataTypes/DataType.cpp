#include "Spire/StyleParser/DataTypes/DataType.hpp"

using namespace Spire;

const std::string& DataType::get_name() const {
  return m_name;
}

bool DataType::operator ==(const DataType& rhs) const {
  if(m_name != rhs.m_name) {
    return false;
  }
  return is_equal(rhs);
}

DataType::DataType(std::string name)
  : m_name(std::move(name)) {}
