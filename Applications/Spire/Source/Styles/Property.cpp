#include "Spire/Styles/Property.hpp"

using namespace Spire;
using namespace Spire::Styles;

std::type_index Property::get_type() const {
  return m_entry->get_type();
}

bool Property::operator ==(const Property& property) const {
  return *m_entry == *property.m_entry;
}

bool Property::operator !=(const Property& property) const {
  return !(*this == property);
}

bool Property::BaseEntry::operator !=(const BaseEntry& entry) const {
  return !(*this == entry);
}
