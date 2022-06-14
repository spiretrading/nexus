#include "Spire/Spire/Field.hpp"

using namespace Spire;

std::size_t Field::get_hash() const {
  return m_accessor->get_hash();
}

bool Field::operator ==(const Field& field) const {
  return *m_accessor == *field.m_accessor;
}

bool Field::operator !=(const Field& field) const {
  return !(*this == field);
}
