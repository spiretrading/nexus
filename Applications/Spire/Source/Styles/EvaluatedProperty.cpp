#include "Spire/Styles/EvaluatedProperty.hpp"

using namespace Spire;
using namespace Spire::Styles;

std::type_index EvaluatedProperty::get_property_type() const {
  return m_entry->get_property_type();
}

Property EvaluatedProperty::as_property() const {
  return m_entry->as_property();
}

bool EvaluatedProperty::operator ==(const EvaluatedProperty& property) const {
  return *m_entry == *property.m_entry;
}

bool EvaluatedProperty::operator !=(const EvaluatedProperty& property) const {
  return !(*this == property);
}
