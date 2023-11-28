#include "Spire/Styles/Property.hpp"
#include <boost/functional/hash.hpp>

using namespace boost;
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

std::size_t std::hash<Property>::operator ()(const Property& property) {
  auto seed = std::size_t(0);
  hash_combine(seed, std::hash<std::type_index>()(property.get_type()));
  hash_combine(seed, property.m_entry->hash());
  return seed;
}
